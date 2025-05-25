use chrono::{SecondsFormat, Utc};
use dotenvy::dotenv;
use serde_json::json;
use std::env;
use std::fs::{create_dir_all, File};
use std::io::Write;
use rand::Rng;

#[derive(Debug)]
struct Entity {
    x: f32,
    y: f32,
    dx: f32,
    dy: f32,
    size: f32,
}

fn main() {
    dotenv().ok();
    let project_root = env::var("PROJECT_ROOT").expect("PROJECT_ROOT not set in .env");
    let data_dir = format!("{}/data", project_root);
    let output_path = format!("{}/input-moving-swarm.json", data_dir);

    create_dir_all(&data_dir).expect("Could not create data directory");

    let mut rng = rand::thread_rng();
    let mut frames = Vec::new();
    let mut entities: Vec<Entity> = Vec::new();

    let base_size = 0.005;
    let steps = 100;
    let max_spawn_per_frame = 5;

    for frame_id in 0..steps {
        let timestamp = Utc::now().to_rfc3339_opts(SecondsFormat::Micros, true);

        // Step 1: Possibly spawn new entities
        let spawn_count = rng.gen_range(0..=max_spawn_per_frame);
        for _ in 0..spawn_count {
            entities.push(Entity {
                x: rng.gen_range(0.0..0.75),
                y: rng.gen_range(0.0..0.75),
                dx: rng.gen_range(-0.01..=0.01),
                dy: rng.gen_range(-0.01..=0.01),
                size: base_size,
            });
        }

        // Step 2: Move and grow all entities
        let mut detections = Vec::new();
        entities.retain_mut(|e| {
            e.x += e.dx;
            e.y += e.dy;
            e.size *= 1.02;

            // Clamp movement speed slowly
            e.dx += rng.gen_range(-0.002..=0.002);
            e.dy += rng.gen_range(-0.002..=0.002);
            e.dx = e.dx.clamp(-0.02, 0.02);
            e.dy = e.dy.clamp(-0.02, 0.02);

            // Check if entity is still on screen
            let within_bounds = e.x >= 0.0 && e.x + e.size <= 1.0 && e.y >= 0.0 && e.y + e.size <= 1.0;

            if within_bounds {
                detections.push(json!({
                    "x": e.x,
                    "y": e.y,
                    "width": e.size,
                    "height": e.size
                }));
            }

            within_bounds
        });

        // Step 3: Store frame
        frames.push(json!({
            "frame_id": frame_id,
            "timestamp": timestamp,
            "detections": detections
        }));
    }

    // Write to file
    let mut file = File::create(&output_path).expect("Could not create file");
    serde_json::to_writer_pretty(&mut file, &frames).expect("Failed to write JSON");

    println!("✅ Multi-entity moving swarm written to {}", output_path);
}
