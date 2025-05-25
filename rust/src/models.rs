use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Deserialize)]
pub struct Detection {
    pub x: f32,
    pub y: f32,
    pub width: f32,
    pub height: f32,
}

#[derive(Debug, Clone, Serialize)]
pub struct TrackedObject {
    pub id: u32,
    pub x: f32,
    pub y: f32,
    pub width: f32,
    pub height: f32,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub history: Option<Vec<(f32, f32)>>, // optional for visual trace
}

#[derive(Debug, Clone, Deserialize)]
pub struct FrameInput {
    pub frame_id: u32,
    pub timestamp: String,
    pub detections: Vec<Detection>,
}

#[derive(Debug, Clone, Serialize)]
pub struct FrameOutput {
    pub frame_id: u32,
    pub timestamp: String,
    pub tracked_objects: Vec<TrackedObject>,
}
