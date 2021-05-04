use std::{fs::OpenOptions, io::Read};


// I didn't use an enum because it makes matching on u8 easier.
#[allow(non_snake_case)]
#[allow(dead_code)]
mod Mnemonic {
    pub const NOOP: u8 = 0x0;  // Immediate ignored
    pub const TOA: u8 = 0x1;   // Immediate is address
    pub const TOB: u8 = 0x2;   // Immediate is address
    pub const SUM: u8 = 0x3;   // Immediate ignored
    pub const SUB: u8 = 0x4;   // Immediate ignored
    pub const FROMA: u8 = 0x5; // Immediate is address
    pub const FROMB: u8 = 0x6; // Immediate is address
    pub const JMP: u8 = 0x7;   // Immediate ignored
    pub const CHNGA: u8 = 0x8; // Immediate has special meaning
    pub const CHNGB: u8 = 0x9; // Immediate has special meaning
    pub const AND: u8 = 0xA;   // Immediate ignored
    // 0xB unused
    // 0xC unused
    // 0xD unused
    pub const JMPC: u8 = 0xE; // Immediate ignored
    pub const JMPZ: u8 = 0xF; // Immediate ignored

                //JMP, JMPC and JMPZ will jump to the address in the next address to them in ram
}
#[derive(Default, Debug)]
#[allow(non_snake_case)]
struct CpuState{
    pub pc: u8,
    pub ir: u8,
    pub mar: u8,
    pub Areg: u8,
    pub Breg: u8,
    pub carry_flag: bool, 
    pub zero_flag: bool
}

impl CpuState{
    fn get_opcode_from_ir(&self) -> u8{
        (self.ir & 0b11110000) >> 4
    }

    fn get_immediate_from_ir(&self) -> u8{
        self.ir & 0b00001111
    }
}

fn main() {
    let mut f = OpenOptions::new().read(true).write(false).create(false).truncate(false).open("/tmp/res.hex").expect("Opening mem file!");
    let mut s = String::new();
    f.read_to_string(&mut s).unwrap();
    let mut memory: Vec<u8> = s.split('\n').into_iter().skip(1).filter(|x|!x.is_empty()).map(|x|u8::from_str_radix(x, 16).unwrap()).collect();

   let mut cpu = CpuState::default();

   loop{
    cpu.mar = cpu.pc;
    cpu.ir = memory[cpu.mar as usize];
    cpu.pc += 1;
    cpu.mar = cpu.pc;

    match cpu.get_opcode_from_ir() {
        Mnemonic::NOOP => {},
        Mnemonic::TOA => {
            cpu.mar = memory[cpu.mar as usize]; // RAMENO | MARLOAD
            cpu.pc += 1; 
            cpu.Areg = memory[cpu.mar as usize]; // RAMENO | ALOAD
        },
        Mnemonic::TOB => {
            cpu.mar = memory[cpu.mar as usize]; // RAMENO | MARLOAD
            cpu.pc += 1;
            cpu.Breg = memory[cpu.mar as usize]; // RAMENO | BLOAD
        },

        Mnemonic::AND => {  // FEN before setting A but after AND-ing
            cpu.carry_flag = cpu.Areg.overflowing_add(cpu.Breg).1;
            let res = cpu.Areg & cpu.Breg;
            cpu.Areg = res;
            cpu.zero_flag = res == 0;
        },
        Mnemonic::SUM => { // FEN before setting A, but after adding
            let (res, c) = cpu.Areg.overflowing_add(cpu.Breg);
            cpu.Areg = res;
            // FEN
            cpu.carry_flag = c;
            cpu.zero_flag = res == 0;
             
        },
        Mnemonic::SUB => { // FEN before setting A, but after subtracting
            cpu.Breg = (-(cpu.Breg as i8)) as u8;
            let (res, c) = cpu.Areg.overflowing_add(cpu.Breg);
            cpu.Areg = res;

            // FEN
            cpu.carry_flag = c;
            cpu.zero_flag = res == 0;
        },
        Mnemonic::FROMA => {
            cpu.mar = memory[cpu.mar as usize]; // RAMENO | MARLOAD
            cpu.pc += 1;
            memory[cpu.mar as usize] = cpu.Areg; // AENO | RAMLOAD
        },
        Mnemonic::FROMB => {
            cpu.mar = memory[cpu.mar as usize]; // RAMENO | MARLOAD
            cpu.pc += 1;
            memory[cpu.mar as usize] = cpu.Breg; // BENO | RAMLOAD
        },
        Mnemonic::JMP => {
            if memory[cpu.mar as usize] == cpu.pc - 1{
                println!("Detected infinite loop, halting!");
                println!("Cpu state: {:?}", cpu);
                println!("Dumping memory:\n {:x?}", memory);
                break;
            }
            cpu.pc = memory[cpu.mar as usize]; // RAMENO | PCLOAD
        },
        Mnemonic::CHNGA => {
            match cpu.get_immediate_from_ir(){
                0x0 => cpu.Areg -= 1,
                0x1 => cpu.Areg += 1,
                0x2 => cpu.Areg = !cpu.Areg,
                0x3 => cpu.Areg = 0,
                0x4 => cpu.Areg = 0xFF,
                0x5 => cpu.Areg &= 0b11111110,
                0x6 => cpu.Areg |= 0b00000001,
                0x7 => cpu.Areg = (-(cpu.Areg as i8)) as u8,
                _ => {panic!("Invalid chng immediate!")}
            } 
            // FEN
            cpu.carry_flag = cpu.Areg.overflowing_add(cpu.Breg).1;
            cpu.zero_flag = cpu.Areg.overflowing_add(cpu.Breg).0 == 0;
        },
        Mnemonic::CHNGB => {
            match cpu.get_immediate_from_ir(){
                0x0 => cpu.Breg -= 1,
                0x1 => cpu.Breg += 1,
                0x2 => cpu.Breg = !cpu.Breg,
                0x3 => cpu.Breg = 0,
                0x4 => cpu.Breg = 0xFF,
                0x5 => cpu.Breg &= 0b11111110,
                0x6 => cpu.Breg |= 0b00000001,
                0x7 => cpu.Breg = (-(cpu.Breg as i8)) as u8,
                _ => {panic!("Invalid chng immediate!")}
            } 
            // FEN
            cpu.carry_flag = cpu.Areg.overflowing_add(cpu.Breg).1;
            cpu.zero_flag = cpu.Areg.overflowing_add(cpu.Breg).0 == 0;
        },
        Mnemonic::JMPC => {
            cpu.pc += 1;
            if cpu.carry_flag {
                cpu.pc = memory[cpu.mar as usize];
            }
        },
        Mnemonic::JMPZ => {
            cpu.pc += 1;
            if cpu.zero_flag {
                cpu.pc = memory[cpu.mar as usize];
            }
        }
        _ => {panic!("Invalid opcode!")}
    }
   }
}
