use std::{io::{Read, BufReader}};
use serialport::{self, DataBits, StopBits, Parity, FlowControl};
use anyhow::Result;

pub enum Instruction {
    Scan,
    Scan4x,
    Scale,
    Park
}

#[derive(Debug)]
pub struct LiDAR {
    vfov: i32,
    hfov: i32,
    dfov: i32,
    cur_x: f32,
    cur_y: f32,
    max_x: f32,
    max_y: f32,
    step_size: f32,
    step_count: i32,
    dir: bool,
    pub port: String,
}

impl LiDAR {
    pub fn new(port: &str, vfov: i32, hfov: i32, dfov: i32) -> Result<Self> {
        Ok(Self {
            vfov,
            hfov,
            dfov,
            port: port.to_string(),
            cur_x: 0.,
            cur_y: 0.,
            max_x: 0.,
            max_y: 0.,
            step_count: 0,
            dir: true,
            step_size: 0.225,
        })
    }

    pub fn start(&mut self, mode: Instruction) -> Result<String> {
        let startx = 90 - self.vfov / 2;
        let stopx = 90 + self.vfov / 2;
        let starty = 90 - self.hfov;
        let stopy = 90 + self.dfov;
        let command = match mode {
            Instruction::Scan => 1,
            Instruction::Scan4x => 2,
            Instruction::Scale => 3,
            Instruction::Park => 4
        };
        // set struct fields accordingly
        self.cur_x = startx as f32;
        self.max_x = stopx as f32;
        self.cur_y = self.hfov as f32;
        self.max_y = self.dfov as f32 * -1.;
        Ok(format!("{} {} {} {} {}\n", command, startx, stopx, starty, stopy))
    }

    // returns is scan is finished
    pub fn step(&mut self) -> Result<bool> {
        if self.cur_y == self.max_y {
            return Ok(true)
        }
        if self.step_count >= (self.vfov as f32/self.step_size) as i32  {
            // println!("[DBG]\tswitched direction at\nX: {}\nY: {}", self.cur_x, self.cur_y);
            // MAYBE here is the problem for not switching?
            // self.cur_x = 90. - self.vfov as f32 / 2.;
            self.cur_y -= 1.;
            self.dir = !self.dir;
            self.step_count = 0;
        }
        self.cur_x += if self.dir {self.step_size} else {self.step_size*-1.};
        self.step_count+=1;
        Ok(false)
    }
}


pub fn configure_port(
    port: &mut Box<dyn serialport::SerialPort>,
    baud_rate: Option<u32>,
    data_bits: Option<DataBits>,
    stop_bits: Option<StopBits>,
    parity_bits: Option<Parity>,
    flow_control: Option<FlowControl>
) -> Result<()> {
    port.set_baud_rate(baud_rate.unwrap_or(115200))?;
    port.set_data_bits(data_bits.unwrap_or(DataBits::Eight))?;
    port.set_parity(parity_bits.unwrap_or(Parity::None))?;
    port.set_stop_bits(stop_bits.unwrap_or(StopBits::One))?;
    port.set_flow_control(flow_control.unwrap_or(FlowControl::None))?;
    Ok(())
}

pub fn read_number<T>(reader: &mut BufReader<T>) -> Result<i32> where T: Read {
    let mut buf = String::new();
    'read: loop {
        let mut buf_c: [u8; 1] = [0;1];
        reader.read_exact(&mut buf_c)?;
        if buf_c[0] == b'\n' {
            break 'read;
        } else {
            buf.push(buf_c[0] as char);
        }
    }
    let result: i32 = buf.trim().parse()?;
    Ok(result)
}

#[allow(non_snake_case)]
pub fn retranslate_angle(lidar: &LiDAR, meas: f32) -> Result<(f32, f32, f32)> {
    let X = lidar.cur_x*-1.;
    let Y = lidar.cur_y;
    let h1 = Y.to_radians().cos() * meas;
    let y = Y.to_radians().sin() * meas;
    let x = X.to_radians().sin() * h1;
    let z = X.to_radians().cos() * h1;
    Ok((x,y,z))
}


// Unit tests
#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn read_number_test () -> Result<()> {
        let a = "234\n3123\n123\n".as_bytes();
        let mut i = BufReader::new(a);
        let num = read_number(&mut i)?;
        assert_eq!(num, 234);
        let num = read_number(&mut i)?;
        assert_eq!(num, 3123);
        let num = read_number(&mut i)?;
        assert_eq!(num, 123);
        Ok(())
    }
}
