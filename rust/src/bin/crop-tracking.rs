use std::fs::File;
use std::io::BufReader;
use std::path::PathBuf;
use std::fs;
use clap::Parser;
use crop_tracking::models::{FrameInput, FrameOutput};
use crop_tracking::tracker::Tracker;
use crop_tracking::visualize::Visualizer;

/// Crop Tracking CLI
#[derive(Parser, Debug)]
#[command(author, version, about)]
struct Args {
    /// Input JSON file
    #[arg(long)]
    input: PathBuf,

    /// Output JSON file
    #[arg(long)]
    output: PathBuf,

    /// Directory to save PNG visualizations
    #[arg(long = "vis-dir")]
    vis_dir: PathBuf,
}

fn main() {
    let args = Args::parse();

    // Read input JSON
    let file = File::open(&args.input).expect("Failed to open input file");
    let reader = BufReader::new(file);
    let frames: Vec<FrameInput> =
        serde_json::from_reader(reader).expect("Failed to parse input JSON");

    let mut tracker = Tracker::new();
    let visualizer = Visualizer::new();
    let mut output_frames: Vec<FrameOutput> = Vec::new();

    for frame in &frames {
        let tracked = tracker.update(&frame.detections);
        let output = FrameOutput {
            frame_id: frame.frame_id,
            timestamp: frame.timestamp.clone(),
            tracked_objects: tracked.clone(),
        };

        visualizer.render_frame(&output, &args.vis_dir);
        output_frames.push(output);
    }

    visualizer.render_summary(&output_frames, &args.vis_dir);

    // Write output JSON
    let json = serde_json::to_string_pretty(&output_frames).expect("Failed to serialize output");
    fs::write(&args.output, json).expect("Failed to write output JSON");
}
