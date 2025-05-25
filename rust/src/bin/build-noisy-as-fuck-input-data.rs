use chrono::{SecondsFormat, Utc};
use dotenvy::dotenv;
use serde_json::json;
use std::env;
use std::fs::{create_dir_all, File};
use std::io::Write;
use rand::Rng;

fn main() {
    // Load environment variables from .env file
    dotenv().ok();

    // Get project root from environment
    let project_root = env::var("PROJECT_ROOT").expect("PROJECT_ROOT not set in .env");

    // Compute data output path
    let data_dir = format!("{}/data", project_root);
    let output_path = format!("{}/input-noisy.json", data_dir);

    // Prepare movement data
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

    create_dir_all(&data_dir).expect("Could not create data directory");

    let mut file = File::create(&output_path).expect("Could not create file");
    serde_json::to_writer_pretty(&mut file, &frames).expect("Failed to write JSON");

    println!("✅ Simple trajectory test data written to {}", output_path);
}
