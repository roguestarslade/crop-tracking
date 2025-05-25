use crate::models::{TrackedObject, FrameOutput};
use std::path::Path;
use tiny_skia::*;
use ab_glyph::{FontArc, PxScale, ScaleFont};
use image::{ImageBuffer, Rgba};
use std::fs;

const IMAGE_WIDTH: u32 = 800;
const IMAGE_HEIGHT: u32 = 800;
const BOX_COLOR: Color = Color::from_rgba8(255, 0, 0, 180);
const TRAIL_COLOR: Color = Color::from_rgba8(0, 200, 255, 128);
const FONT_SIZE: f32 = 18.0;

pub struct Visualizer {
    font: FontArc,
}

impl Visualizer {
    pub fn new() -> Self {
        let font_data = include_bytes!("../fonts/DejaVuSansMono.ttf");
        let font = FontArc::try_from_slice(font_data).expect("Failed to load font");
        Visualizer { font }
    }

    pub fn render_frame(
        &self,
        frame: &FrameOutput,
        output_path: &Path,
    ) {
        let mut pixmap = Pixmap::new(IMAGE_WIDTH, IMAGE_HEIGHT).unwrap();

        for obj in &frame.tracked_objects {
            self.draw_box(&mut pixmap, obj);
            self.draw_id(&mut pixmap, obj);
            self.draw_trail(&mut pixmap, obj);
        }

        // Save image
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
        let rect = Rect::from_xywh(
            obj.x * IMAGE_WIDTH as f32 - obj.width * IMAGE_WIDTH as f32 / 2.0,
            obj.y * IMAGE_HEIGHT as f32 - obj.height * IMAGE_HEIGHT as f32 / 2.0,
            obj.width * IMAGE_WIDTH as f32,
            obj.height * IMAGE_HEIGHT as f32,
        ).unwrap();

        let paint = Paint {
            color: BOX_COLOR,
            ..Default::default()
        };

        let stroke = Stroke {
            width: 2.0,
            ..Default::default()
        };

        pixmap.stroke_rect(rect, &paint, &stroke, Transform::identity(), None);
    }

    fn draw_id(&self, pixmap: &mut Pixmap, obj: &TrackedObject) {
        let scale = PxScale::from(FONT_SIZE);
        let scaled = self.font.as_scaled(scale);

        let text = format!("{}", obj.id);
        let offset_x = obj.x * IMAGE_WIDTH as f32;
        let offset_y = obj.y * IMAGE_HEIGHT as f32 - 10.0;

        for glyph in scaled.layout(&text, ab_glyph::point(offset_x, offset_y)) {
            if let Some(outlined) = scaled.outline_glyph(glyph.clone()) {
                pixmap.fill_path(
                    &outlined,
                    &Paint {
                        color: Color::from_rgba8(255, 255, 255, 255),
                        ..Default::default()
                    },
                    FillRule::Winding,
                    Transform::identity(),
                    None,
                );
            }
        }
    }

    fn draw_trail(&self, pixmap: &mut Pixmap, obj: &TrackedObject) {
        if let Some(history) = &obj.history {
            let mut prev = None;
            for &(x, y) in history {
                let pt = Point::from_xy(x * IMAGE_WIDTH as f32, y * IMAGE_HEIGHT as f32);
                if let Some(p) = prev {
                    let path = PathBuilder::from_path(PathBuilder::new().move_to(p).line_to(pt).finish().unwrap());
                    pixmap.stroke_path(
                        &path,
                        &Paint { color: TRAIL_COLOR, ..Default::default() },
                        &Stroke { width: 1.0, ..Default::default() },
                        Transform::identity(),
                        None,
                    );
                }
                prev = Some(pt);
            }
        }
    }
}
