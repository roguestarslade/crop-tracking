
use std::{env, path::Path};
use std::path::PathBuf;
use dotenvy::dotenv;

fn main() {
    dotenv().ok(); // Load .env
    let project_root = env::var("PROJECT_ROOT").expect("PROJECT_ROOT not set");

    let data_dir = format!("{}/data", project_root);
    let c_dir = format!("{}/c", project_root);
    let rust_dir = format!("{}/rust", project_root);

    println!("📁 Data directory is: {}", data_dir);
}
