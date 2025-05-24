use chrono::Utc;
use rand::Rng;
use serde_json::json;
use std::env;
use std::fs::File;
use std::io::Write;

fn main() {
    // Output path is fixed — no CLI parsing
    let path = "/crop-tracking/data/output-noisy.json";
    let mut rng = rand::thread_rng();
    let mut frames = Vec::new();

    for frame_id in 0..50 {
        let timestamp = Utc::now().to_rfc3339_opts(chrono::SecondsFormat::Micros, true);
        let mut detections = Vec::new();

        let num_objects = rng.gen_range(0..15);
        for _ in 0..num_objects {
            detections.push(json!({
                "x": rng.gen_range(0.0..1.0),
                "y": rng.gen_range(0.0..1.0),
                "width": rng.gen_range(0.01..0.2),
                "height": rng.gen_range(0.01..0.2)
            }));
        }

        frames.push(json!({
            "frame_id": frame_id,
            "timestamp": timestamp,
            "detections": detections
        }));
    }

    std::fs::create_dir_all("/crop-tracking/data").expect("Could not create data directory");

    let mut file = File::create(path).expect("Could not create file");
    serde_json::to_writer_pretty(&mut file, &frames).expect("Failed to write JSON");
    println!("Noisy input data written to {}", path);
}
