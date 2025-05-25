use crate::models::{TrackedObject, FrameOutput};
use std::path::Path;
use std::fs;
use tiny_skia::*;
use ab_glyph::{FontArc, Font, Glyph, PxScale, point};
use image;

pub struct Visualizer {
    font: FontArc,
}

const IMAGE_WIDTH: u32 = 800;
const IMAGE_HEIGHT: u32 = 800;
const FONT_SIZE: f32 = 18.0;

impl Visualizer {
    pub fn new() -> Self {
        let font_data = include_bytes!("../../fonts/DejaVuSansMono.ttf");
        let font = FontArc::try_from_slice(font_data).expect("Failed to load font");
        Visualizer { font }
    }

    pub fn render_frame(&self, frame: &FrameOutput, output_path: &Path) {
        let mut pixmap = Pixmap::new(IMAGE_WIDTH, IMAGE_HEIGHT).unwrap();

        for obj in &frame.tracked_objects {
            self.draw_box(&mut pixmap, obj);
            //self.draw_id(&mut pixmap, obj);
            self.draw_trail(&mut pixmap, obj);
        }

        fs::create_dir_all(output_path).ok();
        let file_path = output_path.join(format!("frame_{:04}.png", frame.frame_id));
        let img = image::RgbaImage::from_raw(
            IMAGE_WIDTH,
            IMAGE_HEIGHT,
            pixmap.data().to_vec(),
        )
        .unwrap();
        img.save(file_path).unwrap();
    }

    pub fn render_summary(&self, all_frames: &[FrameOutput], output_path: &Path) {
        let mut pixmap = Pixmap::new(IMAGE_WIDTH, IMAGE_HEIGHT).unwrap();

        for frame in all_frames {
            for obj in &frame.tracked_objects {
                self.draw_trail(&mut pixmap, obj);
            }
        }

        fs::create_dir_all(output_path).ok();
        let file_path = output_path.join("summary.png");
        let img = image::RgbaImage::from_raw(
            IMAGE_WIDTH,
            IMAGE_HEIGHT,
            pixmap.data().to_vec(),
        )
        .unwrap();
        img.save(file_path).unwrap();
    }

    fn draw_box(&self, pixmap: &mut Pixmap, obj: &TrackedObject) {
        let x = obj.x * IMAGE_WIDTH as f32 - obj.width * IMAGE_WIDTH as f32 / 2.0;
        let y = obj.y * IMAGE_HEIGHT as f32 - obj.height * IMAGE_HEIGHT as f32 / 2.0;
        let w = obj.width * IMAGE_WIDTH as f32;
        let h = obj.height * IMAGE_HEIGHT as f32;

        let rect = Rect::from_xywh(x, y, w, h).unwrap();

        let mut paint = Paint::default();
        //paint.shader = Shader::SolidColor(Color::from_rgba8(255, 0, 0, 180));
        let hash = obj.id.wrapping_mul(2654435761); // Knuth's multiplicative hash
        let r = ((hash >> 0) & 0xFF) as u8;
        let g = ((hash >> 8) & 0xFF) as u8;
        let b = ((hash >> 16) & 0xFF) as u8;
        paint.shader = Shader::SolidColor(Color::from_rgba8(r, g, b, 180));

        let stroke = Stroke {
            width: 2.0,
            ..Default::default()
        };

        let mut pb = PathBuilder::new();
        pb.push_rect(rect);
        let path = pb.finish().unwrap();

        pixmap.stroke_path(&path, &paint, &stroke, Transform::identity(), None);
    }

    fn draw_trail(&self, pixmap: &mut Pixmap, obj: &TrackedObject) {
        if obj.id != 1 {
            return; // Only visualize the main object for now
        }

        if let Some(history) = &obj.history {
            for &(x, y) in history {
                let px = x * IMAGE_WIDTH as f32;
                let py = y * IMAGE_HEIGHT as f32;

                // Draw filled square (6x6 px centered)
                let size = 6.0;
                let rect = Rect::from_xywh(px - size / 2.0, py - size / 2.0, size, size).unwrap();

                let mut paint = Paint::default();
                paint.shader = Shader::SolidColor(Color::from_rgba8(255, 165, 0, 220)); // orange

                pixmap.fill_rect(rect, &paint, Transform::identity(), None);
            }
        }
    }
}
