	.file	"test_i2cmaster.c"
__SP_H__ = 0x3e
__SP_L__ = 0x3d
__SREG__ = 0x3f
__tmp_reg__ = 0
__zero_reg__ = 1
	.text
.Ltext0:
	.cfi_sections	.debug_frame
	.section	.text.startup,"ax",@progbits
.global	main
	.type	main, @function
main:
.LFB0:
	.file 1 "test_i2cmaster.c"
	.loc 1 22 0
	.cfi_startproc
/* prologue: function */
/* frame size = 0 */
/* stack size = 0 */
.L__stack_usage = 0
	.loc 1 26 0
	ldi r24,lo8(-1)
	out 0x17,r24
	.loc 1 27 0
	out 0x18,r24
	.loc 1 29 0
	rcall i2c_init
.LVL0:
	.loc 1 32 0
	ldi r24,lo8(-94)
	rcall i2c_start
.LVL1:
	.loc 1 33 0
	tst r24
	breq .L2
	.loc 1 35 0
	rcall i2c_stop
.LVL2:
	.loc 1 36 0
	out 0x18,__zero_reg__
	rjmp .L3
.LVL3:
.L2:
	.loc 1 39 0
	ldi r24,lo8(5)
.LVL4:
	rcall i2c_write
.LVL5:
	.loc 1 40 0
	ldi r24,lo8(117)
	rcall i2c_write
.LVL6:
	.loc 1 41 0
	rcall i2c_stop
.LVL7:
	.loc 1 45 0
	ldi r24,lo8(-94)
	rcall i2c_start_wait
.LVL8:
	.loc 1 46 0
	ldi r24,lo8(5)
	rcall i2c_write
.LVL9:
	.loc 1 47 0
	ldi r24,lo8(-93)
	rcall i2c_rep_start
.LVL10:
	.loc 1 48 0
	rcall i2c_readNak
.LVL11:
	mov r28,r24
.LVL12:
	.loc 1 49 0
	rcall i2c_stop
.LVL13:
	.loc 1 51 0
	com r28
.LVL14:
	out 0x18,r28
	.loc 1 55 0
	ldi r24,lo8(-94)
	rcall i2c_start_wait
.LVL15:
	.loc 1 56 0
	ldi r24,0
	rcall i2c_write
.LVL16:
	.loc 1 57 0
	ldi r24,lo8(112)
	rcall i2c_write
.LVL17:
	.loc 1 58 0
	ldi r24,lo8(113)
	rcall i2c_write
.LVL18:
	.loc 1 59 0
	ldi r24,lo8(114)
	rcall i2c_write
.LVL19:
	.loc 1 60 0
	ldi r24,lo8(116)
	rcall i2c_write
.LVL20:
	.loc 1 61 0
	rcall i2c_stop
.LVL21:
	.loc 1 65 0
	ldi r24,lo8(-94)
	rcall i2c_start_wait
.LVL22:
	.loc 1 66 0
	ldi r24,0
	rcall i2c_write
.LVL23:
	.loc 1 67 0
	ldi r24,lo8(-93)
	rcall i2c_rep_start
.LVL24:
	.loc 1 68 0
	rcall i2c_readAck
.LVL25:
	.loc 1 69 0
	rcall i2c_readAck
.LVL26:
	.loc 1 70 0
	rcall i2c_readAck
.LVL27:
	.loc 1 71 0
	rcall i2c_readNak
.LVL28:
	mov r28,r24
.LVL29:
	.loc 1 72 0
	rcall i2c_stop
.LVL30:
	.loc 1 74 0
	com r28
.LVL31:
	out 0x18,r28
.LVL32:
.L3:
.L4:
	rjmp .L4
	.cfi_endproc
.LFE0:
	.size	main, .-main
	.text
.Letext0:
	.file 2 "i2cmaster.h"
	.file 3 "/usr/lib/avr/include/stdint.h"
	.section	.debug_info,"",@progbits
.Ldebug_info0:
	.long	0x28b
	.word	0x2
	.long	.Ldebug_abbrev0
	.byte	0x4
	.uleb128 0x1
	.long	.LASF15
	.byte	0xc
	.long	.LASF16
	.long	.LASF17
	.long	.Ldebug_ranges0+0
	.long	0
	.long	0
	.long	.Ldebug_line0
	.uleb128 0x2
	.byte	0x1
	.byte	0x6
	.long	.LASF0
	.uleb128 0x3
	.long	.LASF18
	.byte	0x3
	.byte	0x7e
	.long	0x3b
	.uleb128 0x2
	.byte	0x1
	.byte	0x8
	.long	.LASF1
	.uleb128 0x4
	.byte	0x2
	.byte	0x5
	.string	"int"
	.uleb128 0x2
	.byte	0x2
	.byte	0x7
	.long	.LASF2
	.uleb128 0x2
	.byte	0x4
	.byte	0x5
	.long	.LASF3
	.uleb128 0x2
	.byte	0x4
	.byte	0x7
	.long	.LASF4
	.uleb128 0x2
	.byte	0x8
	.byte	0x5
	.long	.LASF5
	.uleb128 0x2
	.byte	0x8
	.byte	0x7
	.long	.LASF6
	.uleb128 0x5
	.byte	0x1
	.long	.LASF19
	.byte	0x1
	.byte	0x15
	.byte	0x1
	.long	0x42
	.byte	0x1
	.long	.LFB0
	.long	.LFE0
	.byte	0x3
	.byte	0x92
	.uleb128 0x20
	.sleb128 2
	.byte	0x1
	.long	0x226
	.uleb128 0x6
	.string	"ret"
	.byte	0x1
	.byte	0x17
	.long	0x3b
	.long	.LLST0
	.uleb128 0x7
	.long	.LVL0
	.long	0x226
	.uleb128 0x8
	.long	.LVL1
	.long	0x233
	.long	0xb7
	.uleb128 0x9
	.byte	0x1
	.byte	0x68
	.byte	0x2
	.byte	0x9
	.byte	0xa2
	.byte	0
	.uleb128 0x7
	.long	.LVL2
	.long	0x240
	.uleb128 0x8
	.long	.LVL5
	.long	0x24d
	.long	0xd3
	.uleb128 0x9
	.byte	0x1
	.byte	0x68
	.byte	0x1
	.byte	0x35
	.byte	0
	.uleb128 0x8
	.long	.LVL6
	.long	0x24d
	.long	0xe7
	.uleb128 0x9
	.byte	0x1
	.byte	0x68
	.byte	0x2
	.byte	0x8
	.byte	0x75
	.byte	0
	.uleb128 0x7
	.long	.LVL7
	.long	0x240
	.uleb128 0x8
	.long	.LVL8
	.long	0x25a
	.long	0x104
	.uleb128 0x9
	.byte	0x1
	.byte	0x68
	.byte	0x2
	.byte	0x9
	.byte	0xa2
	.byte	0
	.uleb128 0x8
	.long	.LVL9
	.long	0x24d
	.long	0x117
	.uleb128 0x9
	.byte	0x1
	.byte	0x68
	.byte	0x1
	.byte	0x35
	.byte	0
	.uleb128 0x8
	.long	.LVL10
	.long	0x267
	.long	0x12b
	.uleb128 0x9
	.byte	0x1
	.byte	0x68
	.byte	0x2
	.byte	0x9
	.byte	0xa3
	.byte	0
	.uleb128 0x7
	.long	.LVL11
	.long	0x274
	.uleb128 0x7
	.long	.LVL13
	.long	0x240
	.uleb128 0x8
	.long	.LVL15
	.long	0x25a
	.long	0x151
	.uleb128 0x9
	.byte	0x1
	.byte	0x68
	.byte	0x2
	.byte	0x9
	.byte	0xa2
	.byte	0
	.uleb128 0x8
	.long	.LVL16
	.long	0x24d
	.long	0x164
	.uleb128 0x9
	.byte	0x1
	.byte	0x68
	.byte	0x1
	.byte	0x30
	.byte	0
	.uleb128 0x8
	.long	.LVL17
	.long	0x24d
	.long	0x178
	.uleb128 0x9
	.byte	0x1
	.byte	0x68
	.byte	0x2
	.byte	0x8
	.byte	0x70
	.byte	0
	.uleb128 0x8
	.long	.LVL18
	.long	0x24d
	.long	0x18c
	.uleb128 0x9
	.byte	0x1
	.byte	0x68
	.byte	0x2
	.byte	0x8
	.byte	0x71
	.byte	0
	.uleb128 0x8
	.long	.LVL19
	.long	0x24d
	.long	0x1a0
	.uleb128 0x9
	.byte	0x1
	.byte	0x68
	.byte	0x2
	.byte	0x8
	.byte	0x72
	.byte	0
	.uleb128 0x8
	.long	.LVL20
	.long	0x24d
	.long	0x1b4
	.uleb128 0x9
	.byte	0x1
	.byte	0x68
	.byte	0x2
	.byte	0x8
	.byte	0x74
	.byte	0
	.uleb128 0x7
	.long	.LVL21
	.long	0x240
	.uleb128 0x8
	.long	.LVL22
	.long	0x25a
	.long	0x1d1
	.uleb128 0x9
	.byte	0x1
	.byte	0x68
	.byte	0x2
	.byte	0x9
	.byte	0xa2
	.byte	0
	.uleb128 0x8
	.long	.LVL23
	.long	0x24d
	.long	0x1e4
	.uleb128 0x9
	.byte	0x1
	.byte	0x68
	.byte	0x1
	.byte	0x30
	.byte	0
	.uleb128 0x8
	.long	.LVL24
	.long	0x267
	.long	0x1f8
	.uleb128 0x9
	.byte	0x1
	.byte	0x68
	.byte	0x2
	.byte	0x9
	.byte	0xa3
	.byte	0
	.uleb128 0x7
	.long	.LVL25
	.long	0x281
	.uleb128 0x7
	.long	.LVL26
	.long	0x281
	.uleb128 0x7
	.long	.LVL27
	.long	0x281
	.uleb128 0x7
	.long	.LVL28
	.long	0x274
	.uleb128 0x7
	.long	.LVL30
	.long	0x240
	.byte	0
	.uleb128 0xa
	.byte	0x1
	.byte	0x1
	.long	.LASF7
	.long	.LASF7
	.byte	0x2
	.byte	0x66
	.uleb128 0xa
	.byte	0x1
	.byte	0x1
	.long	.LASF8
	.long	.LASF8
	.byte	0x2
	.byte	0x77
	.uleb128 0xa
	.byte	0x1
	.byte	0x1
	.long	.LASF9
	.long	.LASF9
	.byte	0x2
	.byte	0x6d
	.uleb128 0xa
	.byte	0x1
	.byte	0x1
	.long	.LASF10
	.long	.LASF10
	.byte	0x2
	.byte	0x94
	.uleb128 0xa
	.byte	0x1
	.byte	0x1
	.long	.LASF11
	.long	.LASF11
	.byte	0x2
	.byte	0x8b
	.uleb128 0xa
	.byte	0x1
	.byte	0x1
	.long	.LASF12
	.long	.LASF12
	.byte	0x2
	.byte	0x81
	.uleb128 0xa
	.byte	0x1
	.byte	0x1
	.long	.LASF13
	.long	.LASF13
	.byte	0x2
	.byte	0xa1
	.uleb128 0xa
	.byte	0x1
	.byte	0x1
	.long	.LASF14
	.long	.LASF14
	.byte	0x2
	.byte	0x9b
	.byte	0
	.section	.debug_abbrev,"",@progbits
.Ldebug_abbrev0:
	.uleb128 0x1
	.uleb128 0x11
	.byte	0x1
	.uleb128 0x25
	.uleb128 0xe
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x1b
	.uleb128 0xe
	.uleb128 0x55
	.uleb128 0x6
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x52
	.uleb128 0x1
	.uleb128 0x10
	.uleb128 0x6
	.byte	0
	.byte	0
	.uleb128 0x2
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.byte	0
	.byte	0
	.uleb128 0x3
	.uleb128 0x16
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x4
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0x8
	.byte	0
	.byte	0
	.uleb128 0x5
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x87
	.uleb128 0xc
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0xa
	.uleb128 0x2117
	.uleb128 0xc
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x6
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x6
	.byte	0
	.byte	0
	.uleb128 0x7
	.uleb128 0x4109
	.byte	0
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x31
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x8
	.uleb128 0x4109
	.byte	0x1
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x9
	.uleb128 0x410a
	.byte	0
	.uleb128 0x2
	.uleb128 0xa
	.uleb128 0x2111
	.uleb128 0xa
	.byte	0
	.byte	0
	.uleb128 0xa
	.uleb128 0x2e
	.byte	0
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3c
	.uleb128 0xc
	.uleb128 0x2007
	.uleb128 0xe
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_loc,"",@progbits
.Ldebug_loc0:
.LLST0:
	.long	.LVL1
	.long	.LVL2-1
	.word	0x1
	.byte	0x68
	.long	.LVL3
	.long	.LVL4
	.word	0x1
	.byte	0x68
	.long	.LVL12
	.long	.LVL13-1
	.word	0x1
	.byte	0x68
	.long	.LVL13-1
	.long	.LVL14
	.word	0x1
	.byte	0x6c
	.long	.LVL14
	.long	.LVL25
	.word	0x4
	.byte	0x8c
	.sleb128 0
	.byte	0x20
	.byte	0x9f
	.long	.LVL29
	.long	.LVL30-1
	.word	0x1
	.byte	0x68
	.long	.LVL30-1
	.long	.LVL31
	.word	0x1
	.byte	0x6c
	.long	.LVL31
	.long	.LVL32
	.word	0x4
	.byte	0x8c
	.sleb128 0
	.byte	0x20
	.byte	0x9f
	.long	0
	.long	0
	.section	.debug_aranges,"",@progbits
	.long	0x1c
	.word	0x2
	.long	.Ldebug_info0
	.byte	0x4
	.byte	0
	.word	0
	.word	0
	.long	.LFB0
	.long	.LFE0-.LFB0
	.long	0
	.long	0
	.section	.debug_ranges,"",@progbits
.Ldebug_ranges0:
	.long	.LFB0
	.long	.LFE0
	.long	0
	.long	0
	.section	.debug_line,"",@progbits
.Ldebug_line0:
	.section	.debug_str,"MS",@progbits,1
.LASF17:
	.string	"/home/pld/development/pld/pld-usbc-meter/serial-version/firmware"
.LASF8:
	.string	"i2c_start"
.LASF1:
	.string	"unsigned char"
.LASF11:
	.string	"i2c_start_wait"
.LASF13:
	.string	"i2c_readNak"
.LASF4:
	.string	"long unsigned int"
.LASF7:
	.string	"i2c_init"
.LASF10:
	.string	"i2c_write"
.LASF12:
	.string	"i2c_rep_start"
.LASF19:
	.string	"main"
.LASF2:
	.string	"unsigned int"
.LASF6:
	.string	"long long unsigned int"
.LASF18:
	.string	"uint8_t"
.LASF5:
	.string	"long long int"
.LASF9:
	.string	"i2c_stop"
.LASF16:
	.string	"test_i2cmaster.c"
.LASF15:
	.string	"GNU C99 5.4.0 -mn-flash=1 -mno-skip-bug -mmcu=avr25 -gdwarf-2 -Os -std=gnu99 -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums"
.LASF3:
	.string	"long int"
.LASF14:
	.string	"i2c_readAck"
.LASF0:
	.string	"signed char"
	.ident	"GCC: (GNU) 5.4.0"
