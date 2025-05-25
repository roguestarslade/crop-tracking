use crate::models::{Detection, TrackedObject};
use std::collections::HashMap;

const MAX_MISSED_FRAMES: u32 = 3;
const MATCH_DISTANCE_THRESHOLD: f32 = 0.25;

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
            if let Some((track_id, dist)) = self
                .tracks
                .iter()
                .filter(|(_, t)| t.missed < MAX_MISSED_FRAMES)
                .map(|(id, t)| {
                    let d2 = (det.x - t.x).powi(2) + (det.y - t.y).powi(2);
                    (*id, d2.sqrt())
                })
                .filter(|(_, d)| *d < MATCH_DISTANCE_THRESHOLD)
                .min_by(|a, b| a.1.partial_cmp(&b.1).unwrap())
            {
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
                // Create new track
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

        // Update missed counts for unmatched tracks
        for id in unmatched_tracks {
            if let Some(t) = self.tracks.get_mut(&id) {
                t.missed += 1;
            }
        }

        // Prune forgotten tracks
        self.tracks.retain(|_, t| t.missed <= MAX_MISSED_FRAMES);

        assigned_ids
    }
}
