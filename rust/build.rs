
use std::{env, path::Path};

fn main() {
    let proto_dir = match env::var("PROTO_DIR") {
        Ok(val) => val,
        Err(_) => {
            eprintln!("🚨 PROTO_DIR is not set.");
            std::process::exit(1);
        }
    };

    let proto_file = format!("{}/tracked_object.proto", proto_dir);

    eprintln!("🔍 PROTO_DIR = {}", proto_dir);
    eprintln!("🔍 Looking for: {}", proto_file);

    if !Path::new(&proto_file).exists() {
        eprintln!("❌ Cannot find proto file at: {}", proto_file);
        std::process::exit(1);
    }

    println!("cargo:rerun-if-changed={}", proto_file);

    prost_build::compile_protos(&[proto_file], &[proto_dir])
        .expect("💥 Failed to compile tracked_object.proto");
}
