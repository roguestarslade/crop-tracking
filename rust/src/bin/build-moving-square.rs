use chrono::{SecondsFormat, Utc};
use dotenvy::dotenv;
use serde_json::json;
use std::env;
use std::fs::{create_dir_all, File};
use std::io::Write;
use rand::Rng;

fn main() {
    dotenv().ok();
    let project_root = env::var("PROJECT_ROOT").expect("PROJECT_ROOT not set in .env");
    let data_dir = format!("{}/data", project_root);
    let output_path = format!("{}/input-moving-square.json", data_dir);

    create_dir_all(&data_dir).expect("Could not create data directory");

    let mut rng = rand::thread_rng();
    let mut frames = Vec::new();

    let base_size: f32 = 0.05;
    let mut size: f32 = base_size;

    let mut x: f32 = 0.5;
    let mut y: f32 = 0.5;
    let mut dx: f32 = 0.005;
    let mut dy: f32 = 0.004;

    let steps = 100;

    for frame_id in 0..steps {
        let timestamp = Utc::now().to_rfc3339_opts(SecondsFormat::Micros, true);

        // Slightly randomize direction
        dx += rng.gen_range(-0.05..=0.05);
        dy += rng.gen_range(-0.05..=0.05);

        // Slightly randomize size
        size += rng.gen_range(-0.01..=0.01);
        size = size.clamp(base_size, base_size * 3.0);

        // Move square
        x += dx;
        y += dy;

        // Clamp to bounds
        if x < 0.0 || x + size > 1.0 {
            dx = -dx;
            x = x.clamp(0.0, 1.0 - size);
        }
        if y < 0.0 || y + size > 1.0 {
            dy = -dy;
            y = y.clamp(0.0, 1.0 - size);
        }

        let detections = vec![
            json!({
                "x": x,
                "y": y,
                "width": size,
                "height": size
            })
        ];

        frames.push(json!({
            "frame_id": frame_id,
            "timestamp": timestamp,
            "detections": detections
        }));
    }

    let mut file = File::create(&output_path).expect("Could not create file");
    serde_json::to_writer_pretty(&mut file, &frames).expect("Failed to write JSON");

    println!("✅ Moving square trajectory written to {}", output_path);
}
