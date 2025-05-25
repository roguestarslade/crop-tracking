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
        paint.shader = Shader::SolidColor(Color::from_rgba8(255, 0, 0, 180));

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
        if let Some(history) = &obj.history {
            let mut prev: Option<(f32, f32)> = None;
            for &(x, y) in history {
                let px = x * IMAGE_WIDTH as f32;
                let py = y * IMAGE_HEIGHT as f32;
                if let Some((last_x, last_y)) = prev {
                    let mut pb = PathBuilder::new();
                    pb.move_to(last_x, last_y);
                    pb.line_to(px, py);
                    let path = pb.finish().unwrap();

                    let mut paint = Paint::default();
                    paint.shader = Shader::SolidColor(Color::from_rgba8(0, 200, 255, 128));

                    let stroke = Stroke {
                        width: 1.0,
                        ..Default::default()
                    };

                    pixmap.stroke_path(&path, &paint, &stroke, Transform::identity(), None);
                }
                prev = Some((px, py));
            }
        }
    }
}
