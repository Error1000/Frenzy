use std::{collections::HashMap, convert::identity, io::Read, str::FromStr};

use nom::combinator::rest;
use nom::sequence::tuple;
use nom::{
    branch::alt,
    bytes::complete::{tag, take_while, take_while1},
    character::complete::{alpha1, digit1, space0, space1},
    combinator::{eof, opt},
    AsChar, IResult, Parser,
};
use nom::{character::complete::char, combinator::recognize};

fn consume_comment(input: &str) -> IResult<&str, ()> {
    let (input, _) = tuple((char(';'), rest))(input)?;
    Ok((input, ()))
}

fn consume_label(input: &str) -> IResult<&str, &str> {
    recognize(tuple((
        take_while1(|x: char| x.is_alpha() || x == '_'),
        take_while(|x: char| x.is_alphanum() || x == '_'),
    )))(input)
}

/******************************************************/

// Consumes entire input and tries to parse it to u8
fn str_to_u8(snum: &str, radix: u32) -> IResult<&str, u8> {
    let r = u8::from_str_radix(snum, radix).map_err(|_| {
        nom::Err::Failure(nom::error::Error::new(
            "Failed parsing binary number!",
            nom::error::ErrorKind::Digit,
        ))
    })?;
    Ok(("", r))
}

fn parse_bin_number(input: &str) -> IResult<&str, u8> {
    let (input, (_, snum)) = tuple((tag("0b"), digit1))(input)?;
    let (_, num) = str_to_u8(snum, 2)?;
    Ok((input, num))
}

fn parse_hex_number(input: &str) -> IResult<&str, u8> {
    let (input, (_, snum)) = tuple((tag("0x"), digit1))(input)?;
    let (_, num) = str_to_u8(snum, 16)?;
    Ok((input, num))
}

fn parse_dec_number(input: &str) -> IResult<&str, u8> {
    let (input, snum) = digit1(input)?;
    let (_, num) = str_to_u8(snum, 10)?;
    Ok((input, num))
}

fn parse_number(input: &str) -> IResult<&str, u8> {
    alt((parse_hex_number, parse_bin_number, parse_dec_number))(input)
}

/******************************************************/

fn parse_addr(input: &str) -> IResult<&str, u8> {
    let (input, (addr, _)) = tuple((parse_number, char(':')))(input)?;
    Ok((input, addr))
}

fn parse_mnemonic(input: &str) -> IResult<&str, Mnemonic> {
    let (input, mn) = alpha1(input)?;
    Ok((
        input,
        Mnemonic::from_str(mn).map_err(|_| {
            nom::Err::Failure(nom::error::Error::new(
                "Failed parsing mnemonic!",
                nom::error::ErrorKind::Alpha,
            ))
        })?,
    ))
}

fn parse_ins_data(input: &str) -> IResult<&str, InsData> {
    alt((
        parse_number.map(|x| InsData::Data(x)),
        consume_label_use.map(|x| InsData::LabelUse(x)),
    ))(input)
}

fn parse_instruction(input: &str) -> IResult<&str, Instruction> {
    let (input, (mnemonic, immediate)) = tuple((
        parse_mnemonic, // First a mnemonic
        // Then at least one space and an InsData
        opt(tuple((space1, parse_ins_data))).map(|x| x.map(|x| x.1)),
    ))(input)?;
    Ok((
        input,
        Instruction {
            name: mnemonic,
            immediate,
        },
    ))
}

fn end_line(input: &str) -> IResult<&str, ()> {
    let (input, _) = tuple((space0, opt(consume_comment), eof))(input)?;
    Ok((input, ()))
}

fn parse_definition<'a>(input: &'a str) -> IResult<&'a str, Definition<'a>> {
    alt((
        tuple((consume_label, char(':'), space0, parse_addr))
            .map(|x| Definition::LabelDefAndAddr(x.0, x.3)),
        tuple((parse_addr, space0, consume_label)).map(|x| Definition::AddrAndLabelDef(x.0, x.2)),
        tuple((consume_label, char(':'))).map(|x| Definition::LabelDef(x.0)),
        parse_addr.map(|x| Definition::Addr(x)),
    ))(input)
}

fn consume_label_use(input: &str) -> IResult<&str, &str> {
    let (input, (_, label, _)) = tuple((char('{'), consume_label, char('}')))(input)?;
    Ok((input, label))
}

fn parse_line<'a>(line: &'a str) -> Option<LineResult<'a>> {
    alt((
        // Only definition
        tuple((space0, parse_definition, end_line)).map(|x| LineResult::Definition(x.1)),
        
        // Statement and maybe definition
        tuple((
            space0,
            opt(parse_definition),
            space0,
            alt((
                parse_instruction.map(|x| Statement::Inst(x)),
                consume_label_use.map(|x| Statement::LabelUse(x)),
                parse_number.map(|x| Statement::Raw(x)),
            )),
            end_line,
        ))
        .map(|x| LineResult::Statement(x.1, x.3)),
    ))(line)
    .map(|x| x.1)
    .ok()
}

fn parser<'a>(input: impl Iterator<Item = &'a str>) -> impl Iterator<Item = LineResult<'a>> {
    input.map(parse_line).filter_map(identity)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_definition() {
        let (_, res) = parse_definition("HELLO: 0x9: ").unwrap();
        assert_eq!(res, Definition::LabelDefAndAddr("HELLO", 0x9));

        let (_, res) = parse_definition("0b101: YO: ").unwrap();
        assert_eq!(res, Definition::AddrAndLabelDef(0b101, "YO"));

        let (_, res) = parse_definition("MY_MAN2:").unwrap();
        assert_eq!(res, Definition::LabelDef("MY_MAN2"));
    }

    #[test]
    fn test_number() {
        let (_, res) = parse_number("123").unwrap();
        assert_eq!(res, 123);
        let (_, res) = parse_number("0x10").unwrap();
        assert_eq!(res, 16);
        let (_, res) = parse_number("0b101").unwrap();
        assert_eq!(res, 5);
    }

    #[test]
    fn test_inst() {
        let (_, res) = parse_instruction("CHNGA 0x0").unwrap();
        assert_eq!(
            res,
            Instruction {
                name: Mnemonic::CHNGA,
                immediate: Some(InsData::Data(0))
            }
        );
        let (_, res) = parse_instruction("NOOP").unwrap();
        assert_eq!(
            res,
            Instruction {
                name: Mnemonic::NOOP,
                immediate: None
            }
        );
    }

    #[test]
    fn test_lables() {
        let (_, res) = consume_label_use("{HELLO}").unwrap();
        assert_eq!(res, "HELLO");
    }
}

#[derive(Debug, Copy, Clone, PartialEq)]
#[repr(u8)]
enum Mnemonic {
    NOOP = 0x0,  // Immediate ignored
    TOA = 0x1,   // Immediate is address
    TOB = 0x2,   // Immediate is address
    SUM = 0x3,   // Immediate ignored
    SUB = 0x4,   // Immediate ignored
    FROMA = 0x5, // Immediate is address
    FROMB = 0x6, // Immediate is address
    JMP = 0x7,   // Immediate ignored
    CHNGA = 0x8, // Immediate has special meaning
    CHNGB = 0x9, // Immediate has special meaning
    AND = 0xA,   // Immediate ignored
    // 0xB unused
    // 0xC unused
    // 0xD unused
    JMPC = 0xE, // Immediate ignored
    JMPZ = 0xF, // Immediate ignored

                //JMP, JMPC and JMPZ will jump to the address in the next address to them in ram
}

impl FromStr for Mnemonic {
    type Err = ();

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        Ok(match s {
            "NOOP" => Mnemonic::NOOP,
            "TOA" => Mnemonic::TOA,
            "TOB" => Mnemonic::TOB,
            "SUM" => Mnemonic::SUM,
            "SUB" => Mnemonic::SUB,
            "FROMA" => Mnemonic::FROMA,
            "FROMB" => Mnemonic::FROMB,
            "JMP" => Mnemonic::JMP,
            "CHNGA" => Mnemonic::CHNGA,
            "CHNGB" => Mnemonic::CHNGB,
            "AND" => Mnemonic::AND,
            "JMPC" => Mnemonic::JMPC,
            "JMPZ" => Mnemonic::JMPZ,
            _ => return Err(()),
        })
    }
}

#[derive(Debug, Copy, Clone, PartialEq)]
enum InsData<'a> {
    LabelUse(&'a str),
    Data(u8),
}

#[derive(Debug, Copy, Clone, PartialEq)]
struct Instruction<'a> {
    name: Mnemonic,
    immediate: Option<InsData<'a>>,
}

// A definitions is all the stuff before the last ':' character on a line
#[derive(Debug, PartialEq)]
enum Definition<'a> {
    LabelDefAndAddr(&'a str, u8),
    AddrAndLabelDef(u8, &'a str),
    Addr(u8),
    LabelDef(&'a str),
}

#[derive(Debug, PartialEq)]
enum Statement<'a> {
    Inst(Instruction<'a>),
    LabelUse(&'a str),
    Raw(u8),
}

// Only statements should increase the current address
#[derive(Debug, PartialEq)]
enum LineResult<'a> {
    Definition(Definition<'a>),
    Statement(Option<Definition<'a>>, Statement<'a>),
}

fn main() {
    let mut f = {
        let fpath = std::env::args().skip(1).next().expect("Need the file path as first argument!");
        std::fs::OpenOptions::new().read(true).write(false).open(fpath).expect("File path should be valid!")
    };
    let mut data = String::new();
    f.read_to_string(&mut data).unwrap();

    let mut lables: HashMap<&str, u8> = HashMap::new();
    let mut statements: Vec<(Statement, u8)> = Vec::new();
{
    let mut curr_addr: u8 = 0;
    for parsed_line in parser(data.split('\n')) {
        let (def, s) = match parsed_line{
            LineResult::Definition(d) => (Some(d), None),
            LineResult::Statement(d, s) => (d, Some(s))
        };

        // Parse definition
        if let Some(def) = def {
            match def{
                Definition::LabelDef(l) => {lables.insert(l, curr_addr);},
                Definition::AddrAndLabelDef(a, l) => {curr_addr = a; lables.insert(l, curr_addr);},
                Definition::LabelDefAndAddr(l, a) => {lables.insert(l, curr_addr); curr_addr = a;},
                Definition::Addr(a) => {curr_addr = a;},
            }
        }

        // Save statement for later parsing after we finish with definitions
        if let Some(s) = s{
            statements.push((s, curr_addr));
            curr_addr += 1;
        }
    }
}

    // Now that we parsed all definitions parse statements
    for (s, addr) in statements{
        // Resolve inst_data to raw values
        let resolved = match s {
            Statement::Raw(d) => d,
            Statement::Inst(ins) => {
                if let Some(imm) = ins.immediate{
                    match imm{
                        InsData::Data(d) => ((ins.name as u8) << 4) | d,
                        InsData::LabelUse(lbl) => ((ins.name as u8) << 4) | lables[lbl]
                    }
                }else{
                    (ins.name as u8) << 4 | 0
                }
            },
            Statement::LabelUse(lbl) => lables[lbl]
        };
        println!("{:#x}: {:#x}", addr, resolved);
    }
}
