fn main() {
    prost_build::compile_protos(
        &["../proto/tracked_object.proto"],
        &["../proto"],
    ).unwrap();
}
