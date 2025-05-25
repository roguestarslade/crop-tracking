use crate::models::{Detection, TrackedObject};
use std::collections::HashMap;

const MAX_MISSED_FRAMES: u32 = 3;
const MATCH_DISTANCE_THRESHOLD: f32 = 0.05;
const SIZE_SIMILARITY_THRESHOLD: f32 = 0.05;

#[derive(Debug)]
struct TrackEntry {
    id: u32,
    x: f32,
    y: f32,
    width: f32,
    height: f32,
    missed: u32,
    history: Vec<(f32, f32)>,
}

pub struct Tracker {
    next_id: u32,
    tracks: HashMap<u32, TrackEntry>,
}

impl Tracker {
    pub fn new() -> Self {
        Tracker {
            next_id: 1,
            tracks: HashMap::new(),
        }
    }

    pub fn update(&mut self, detections: &[Detection]) -> Vec<TrackedObject> {
        let mut assigned_ids = Vec::new();
        let mut unmatched_tracks: Vec<u32> = self.tracks.keys().cloned().collect();

        for det in detections {
            let mut best_match: Option<(u32, f32)> = None;

            for (&track_id, track) in self.tracks.iter().filter(|(_, t)| t.missed < MAX_MISSED_FRAMES) {
                let dx = det.x - track.x;
                let dy = det.y - track.y;
                let dw = (det.width - track.width).abs();
                let dh = (det.height - track.height).abs();

                let distance = (dx * dx + dy * dy).sqrt();
                let size_diff = dw + dh;

                if distance < MATCH_DISTANCE_THRESHOLD && size_diff < SIZE_SIMILARITY_THRESHOLD {
                    match best_match {
                        Some((_, best_dist)) if distance < best_dist => {
                            best_match = Some((track_id, distance));
                        }
                        None => {
                            best_match = Some((track_id, distance));
                        }
                        _ => {}
                    }
                }
            }

            if let Some((track_id, _)) = best_match {
                let track = self.tracks.get_mut(&track_id).unwrap();
                track.x = det.x;
                track.y = det.y;
                track.width = det.width;
                track.height = det.height;
                track.missed = 0;
                track.history.push((det.x, det.y));

                assigned_ids.push(TrackedObject {
                    id: track.id,
                    x: det.x,
                    y: det.y,
                    width: det.width,
                    height: det.height,
                    history: Some(track.history.clone()),
                });

                unmatched_tracks.retain(|id| *id != track_id);
            } else {
                // New track
                let id = self.next_id;
                self.next_id += 1;

                self.tracks.insert(
                    id,
                    TrackEntry {
                        id,
                        x: det.x,
                        y: det.y,
                        width: det.width,
                        height: det.height,
                        missed: 0,
                        history: vec![(det.x, det.y)],
                    },
                );

                assigned_ids.push(TrackedObject {
                    id,
                    x: det.x,
                    y: det.y,
                    width: det.width,
                    height: det.height,
                    history: Some(vec![(det.x, det.y)]),
                });
            }
        }

        // Update missed counters
        for id in unmatched_tracks {
            if let Some(t) = self.tracks.get_mut(&id) {
                t.missed += 1;
            }
        }

        // Prune old tracks
        self.tracks.retain(|_, t| t.missed <= MAX_MISSED_FRAMES);

        assigned_ids
    }
}
