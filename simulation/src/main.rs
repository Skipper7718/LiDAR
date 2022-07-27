use anyhow::Result;
use kiss3d::{light, window, nalgebra::{Point3, OPoint}};
use std::{fs::File, env, process, io::{BufReader}};
// use clap::{App, Arg, Subcommand};
pub mod lidar;

const WINDOW_TITLE: &str = "LiDAR Live View";
const FRAMERATE: u64 = 1000;

fn main() -> Result<()> {
    /*TODO implement clap Argarse */

    if !cfg!(debug_assertions) {
        println!("Not debug build... running in demo mode");
        println!("Autostarting scan simulation from file stream");
        let vfov: i32 = 162;
        let hfov: i32 = 40;
        let dfov: i32 = 10;
        let filename = "./c2_stripped.ostream";
        let argv: Vec<String> = format!("sim file {} {} {} {}", filename, vfov, hfov, dfov).split(' ')
        .map(|s| s.to_string())
        .collect();
        main_file(&argv)?;
        return Ok(())
    }
    // main_serial()?;
    let argv: Vec<String> = env::args().into_iter().collect();
    if argv.len() < 6 {
        usage(&argv);
    }
    match argv[1].as_str() {
        "file" => main_file(&argv)?,
        "serial" => main_serial(&argv)?,
        _ => usage(&argv)
    }
    Ok(())
}


//Made for serial mode
// connect to lidar and see a live scan process
fn main_serial(argv: &[String]) -> Result<()> {

    let mut li = lidar::LiDAR::new(argv[2].as_str(), 180,20, 10)?;
    let mut port = serialport::new(&li.port, 115200).open()?;
    lidar::configure_port(&mut port, None, None, None, None, None)?;

    let command = li.start(lidar::Instruction::Scan)?;
    let _ = port.write(command.as_bytes())?;
    port.flush()?;

    unimplemented!();
    // let reader = BufReader::new(port);
    // Discontinued bcs this project idea is shit.
    // The scanning is already in the main program
}


// scan from file
// for demo modes the release build defaults to a specific file with settings
fn main_file(argv: &[String]) -> Result<()> {
    // "./c2_stripped.ostream", 162, 40, 10
    let filename = &argv[2];
    let vfov: i32 = argv[3].parse()?;
    let hfov: i32 = argv[4].parse()?;
    let dfov: i32 = argv[5].parse()?;

    let mut li = lidar::LiDAR::new(filename.as_str(), vfov,hfov,dfov)?;
    let port = File::open(&li.port)?;

    // window setup
    let mut window = window::Window::new(WINDOW_TITLE);
    let point_color = Point3::new(1.,1.,1.);
    let line_color = Point3::new(0.,1.,0.5);
    window.set_framerate_limit(Some(FRAMERATE));
    window.set_light(light::Light::StickToCamera);

    let mut reader = BufReader::new(port);
    // let mut points: Vec<(f32, f32, f32)> = Vec::new();
    let mut points: Vec<OPoint<f32, _>> = Vec::new();
    let command = li.start(lidar::Instruction::Scan)?;
    println!("Starting with command: {}", command);
    
    while window.render() {
        // kiss 3d is having problems with rendering more than 300 fps.
        // specify how many points will be rendered in a frame
        for _ in 0..25 {
            if !li.step()? {
                let num = lidar::read_number(&mut reader).unwrap_or(0);
                let point = lidar::retranslate_angle(&li, num as f32)?;
                points.push(Point3::new(point.0/100., point.1/100., point.2/100.));
            }
        }
        for point in &points {
            window.draw_point(point, &point_color);
        }
        window.draw_line(&Point3::new(0.,0.,0.), &points[points.len()-1], &line_color);
    }

    Ok(())
}

fn usage(argv: &[String]) {
    println!("------------------------------");
    println!("----------- USAGE ------------");
    println!();
    println!("-- {} serial [port vfov hfov dfov] --", &argv[0]);
    println!("-- {} file [filename vfov hfov dfov]", &argv[0]);
    println!();
    println!("------------------------------");
    process::exit(1);
}
