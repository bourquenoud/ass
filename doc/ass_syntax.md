***DISCAIMER*** *: not done yet, this is just information thrown in without sorting or anything*

***INFO*** *: name between `<` and `>` mean they are to be replaced with their corresponding value*

# Syntax

## General syntax

<u>*Comments*</u> are in the form of ` //<comment> ` for single lines and ` /*<comment>*/ ` for multi-lines.

<u>*Integers*</u> are constants used outside of bit patterns. They are in base 10 (decimal). Example : `10 423 -84`

<u>*Strings*</u> are constants used outside of bit patterns. They are enclosed in double quotes `""`, which can be escaped with a backslash `\`. Example : `"This is a string" "ab" "" "\"escaped quote"`

## Command

<u>*Commands*</u> start with a percent `%`.

Example :

```
%param opcode-length 15
%enum reg 4
```

## Substitution

<u>*Substitutions*</u> are in all caps, they used for ...blabla... bit length in parenthesis.

Example :

```
%format jmp_format [$0010,reg,reg,IMMEDIATE(8)]    // IMMEDIATE(8) will be substituted for an immediate value of 8 bits during assembly
%format jmp_format [reg,LABEL_REL(10),io,..]       // LABEL_REL(10) will be substituted for an offset with computed from a label name
```

## Enumeration

<u>*Enumeration*</u> start with a letter in small caps, and can use any letter or number, or an underscore after that. Enumeration pattern are enclosed in double quotes `"`, they can use any letter, numbers or symbols except white spaces. Double quotes `"` need to be escaped with a backslash `\`. Enumerations are not checked against pattern collison (for now). Use the directive 

Example:

```
%enum reg 4                 // Declaring an enum called "reg" of a lenght of 4 bits
%pattern reg "ax" $1001     // Anytime the assembler find "ax" where a reg enum is expected, it will replace the 4 bits with 1001
%pattern reg "bx" $1011     // Anytime the assembler find "bx" where a reg enum is expected, it will replace the 4 bits with 1011
%pattern reg "cx" $1111     // Anytime the assembler find "cx" where a reg enum is expected, it will replace the 4 bits with 1101
```

## Bit constant

<u>*Bit constants*</u> start with an `$`. With no postfix the value is considered to be in base 2, postfixes of 'h', 'd', 'o', 'b' respectively mean hexadecimal, decimal, octal, binary. You can either use a numerical value, or define a constant using the directive `%constant`. Defined constants have to be written in small caps and cannot include numbers or symbols except underscodes `_`, but they cannot start with an underscore. They are defined without a leading dollars `$` but needs it when used.

Example:

```
%constant cnt $541Fh                  // Define a constant value of 0x541F
%constant format_0 $0010              // Define a constant value of 0b0010
%format rrr [$cnt,$32d,$format_0]     // Equivalent to "%format rrr [$541Fh,$32d,$0010]"
```

## Bit pattern

<u>*Bit patterns*</u> are enclosed in square brackets `[]`. You can use a constant, a substitution or an enumeration, they must be separated by a comma `,`. The bit pattern must be of exactly the correct size, or be filled with 0 using an ellipis `...`. 

Examples :
```
%format cmp_ld_type [$0010,reg,reg,IMMEDIATE(8)]    // opcode using this format will produce a word starting with 0010, then two registers and an imediate
%format jmp_format [$001,LABEL_ABS(16),...]         // 
```

<u>*Opcode formats*</u> are declared using the directive `%format`.

<u>*Opcodes*</u> or mnemonics are declared using the directive `%opcode`.

<u>**</u>

# Directives



# Parameters

Parameter for ASS are declared like this :

```
%param <parameter-name> <parameter-argument>,<parameter-argument>,...
```

List of parameters :

| Mandatory | Name                           | Argument                         | Description                    |
| :-------: | ------------------------------ | -------------------------------- | ------------------------------ |
|    [X]    | `opcode-width <width>`         | width (integer)                  | Configure the width of the opcodes in bits. This value is fixed, meaning you can not have variable lenght opcodes|
|    [X]    | `address-width <width>`        | width (integer)                  | Configure the width of the address bus. |
|    [ ]    | `separators <seprator list>`   | separator list (string)          | Define the separtor used betweem arguments for mnemonics. By default they are separated by a space. You can define multiple separators.
