use chrono::Utc;
use serde_json::json;
use std::env;
use std::fs::File;
use std::io::Write;

fn main() {
    //test commit from Docker container

    // Output path is fixed — no CLI parsing
    let path = "/crop-tracking/data/output-simple.json";
    let mut frames = Vec::new();
    let square_size = 0.05;
    let steps = 50;

    for frame_id in 0..steps {
        let t = frame_id as f32 / (steps - 1) as f32;
        let timestamp = Utc::now().to_rfc3339_opts(chrono::SecondsFormat::Micros, true);

        let detections = vec![
            // 1. Left to Right
            json!({ "x": t, "y": 0.1, "width": square_size, "height": square_size }),
            // 2. Top to Bottom
            json!({ "x": 0.1, "y": t, "width": square_size, "height": square_size }),
            // 3. Top-Left to Bottom-Right
            json!({ "x": t, "y": t, "width": square_size, "height": square_size }),
            // 4. Top-Right to Bottom-Left
            json!({ "x": 1.0 - t, "y": t, "width": square_size, "height": square_size }),
            // 5. Right to Left
            json!({ "x": 1.0 - t, "y": 0.9, "width": square_size, "height": square_size }),
            // 6. Bottom to Top
            json!({ "x": 0.9, "y": 1.0 - t, "width": square_size, "height": square_size }),
            // 7. Top to Bottom (again)
            json!({ "x": 0.5, "y": t, "width": square_size, "height": square_size }),
            // 8. Bottom-Right to Top-Left
            json!({ "x": 1.0 - t, "y": 1.0 - t, "width": square_size, "height": square_size }),
            // 9. Bottom-Left to Top-Right
            json!({ "x": t, "y": 1.0 - t, "width": square_size, "height": square_size }),
        ];

        frames.push(json!({
            "frame_id": frame_id,
            "timestamp": timestamp,
            "detections": detections
        }));
    }

    std::fs::create_dir_all("/crop-tracking/data").expect("Could not create data directory");

    let mut file = File::create(path).expect("Could not create file");
    serde_json::to_writer_pretty(&mut file, &frames).expect("Failed to write JSON");
    println!("Simple trajectory test data written to {}", path);
}
