const DEF: usize = 0b00001;
const INMOD: usize = 0b00010;
const LOAD: usize = 0b00100;
const CLKMOD: usize = 0b01000;
const CASCMOD: usize = 0b10000;

struct Step3(usize);

struct Step4(usize);

const DEC: usize = 0;
const INC: usize = 1;
const INV: usize = 2;
const SET_0: usize = 3;
const SET_FF: usize = 4;
const SETLB0: usize = 5;
const SETLB1: usize = 6;
const NEG: usize = 7;

const ADDRES_SIZE: usize = 4;

#[allow(unused_parens)]
const ROM_SIZE: usize = (1 << ADDRES_SIZE);

struct Rom {
    arr: [usize; ROM_SIZE],
}

impl Rom {
    fn set(&mut self, label: usize, s3: Step3, s4: Step4) {
        self.arr[label] = s3.0;
        self.arr[label | 0b1000] = s4.0;
    }
}

/* 
    This maps the bus to control lines
    when the CNTRLLINEMOD control line is on(high)
*/

fn main() {
    let mut r: Rom = Rom { arr: [0; ROM_SIZE] };

    r.set(
        DEC,
        	Step3(DEF | INMOD | CLKMOD | CASCMOD),
        	Step4(DEF | INMOD | CLKMOD | CASCMOD | LOAD),
    );

    r.set(
        INC,
	        Step3(DEF | INMOD | CLKMOD),
        	Step4(DEF | INMOD | CLKMOD | LOAD),
    );

    r.set(
	INV, 
		Step3(DEF | INMOD), 
		Step4(DEF | INMOD | LOAD)
    );

    r.set(
	SET_0,
		Step3(0),
		Step4(LOAD)
    );

    r.set(
	SET_FF, 
		Step3(DEF), 
		Step4(DEF | LOAD)
    );

    r.set(
        SETLB0,
	        Step3(CASCMOD | CLKMOD),
        	Step4(CASCMOD | CLKMOD | LOAD),
    );

    r.set(
	SETLB1,
		Step3(CLKMOD | DEF),
		Step4(CLKMOD | DEF | LOAD)
    );

    r.set(
        NEG,
        	Step3(DEF | LOAD | INMOD),
        	Step4(DEF | LOAD | INMOD | CLKMOD),
    );

    for i in 0..ROM_SIZE {
        println!("{:#X}: {:#X}", i, r.arr[i]);
    }
}
