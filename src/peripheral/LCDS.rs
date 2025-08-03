use std::error::Error;
use rppal::spi::{Bus, Mode, Segment, SlaveSelect, Spi};
use log::{info, warn, error, debug};
/* 
    RASPBERRY CODES
 */
// Instruction set.
const WRITE: u8 = 0b0010; // Write data, starting at the selected address.
const READ: u8 = 0b0011; // Read data, starting at the selected address.
const RDSR: u8 = 0b0101; // Read the STATUS register.
const WREN: u8 = 0b0110; // Set the write enable latch (enable write operations).
const WIP: u8 = 1; // Write-In-Process bit mask for the STATUS register.


/*
    DIGILENT CODES
*/
// Command constants
const ESC: u8 = 0x1B;
const BRACKET: u8 = 0x5B; // [
const CURSOR_POS_CMD: u8 = 0x48; // H
const CURSOR_SAVE_CMD: u8 = 0x73; // s
const CURSOR_RSTR_CMD: u8 = 0x75; // u
const DISP_CLR_CMD: u8 = 0x6A; // j
const ERASE_INLINE_CMD: u8 = 0x4B; // K
const ERASE_FIELD_CMD: u8 = 0x4E; // N
const LSCROLL_CMD: u8 = 0x40; // @
const RSCROLL_CMD: u8 = 0x41; // A
const RST_CMD: u8 = 0x2A; // *
const DISP_EN_CMD: u8 = 0x65; // e
const DISP_MODE_CMD: u8 = 0x68; // h
const CURSOR_MODE_CMD: u8 = 0x63; // c
const TWI_SAVE_ADDR_CMD: u8 = 0x61; // a
const BR_SAVE_CMD: u8 = 0x62; // b
const PRG_CHAR_CMD: u8 = 0x70; // p
const SAVE_RAM_TO_EEPROM_CMD: u8 = 0x74; // t
const LD_EEPROM_TO_RAM_CMD: u8 = 0x6C; // l
const DEF_CHAR_CMD: u8 = 0x64; // d
const COMM_MODE_SAVE_CMD: u8 = 0x6D; // m
const EEPROM_WR_EN_CMD: u8 = 0x77; // w
const CURSOR_MODE_SAVE_CMD: u8 = 0x6E; // n
const DISP_MODE_SAVE_CMD: u8 = 0x6F; // o

// Access parameters for communication ports
const PAR_ACCESS_DSPI0: u8 = 0;
const PAR_ACCESS_DSPI1: u8 = 1;
const PAR_SPD_MAX: u32 = 625_000;

// Error definitions
const LCDS_ERR_SUCCESS: u8 = 0;
const LCDS_ERR_ARG_ROW_RANGE: u8 = 1;
const LCDS_ERR_ARG_COL_RANGE: u8 = 2;
const LCDS_ERR_ARG_ERASE_OPTIONS: u8 = 3;
const LCDS_ERR_ARG_BR_RANGE: u8 = 4;
const LCDS_ERR_ARG_TABLE_RANGE: u8 = 5;
const LCDS_ERR_ARG_COMM_RANGE: u8 = 6;
const LCDS_ERR_ARG_CRS_RANGE: u8 = 7;
const LCDS_ERR_ARG_DSP_RANGE: u8 = 8;
const LCDS_ERR_ARG_POS_RANGE: u8 = 9;

// Other defines
const MAX: usize = 150;

pub struct LCDS {
    spi_module: Option<Spi>,
}

impl LCDS {
    /// Creates a new LCDS instance with no SPI module initialized.
    pub fn new() -> Self {
        Self {
            spi_module: None,
        }
    }

    /// Initializes the SPI interface with the given parameters.
    ///
    /// # Arguments
    /// * `bus` - The SPI bus to use (e.g., Bus::Spi0).
    /// * `slave_select` - The slave select line.
    /// * `clock_speed` - The SPI clock speed in Hz.
    /// * `mode` - The SPI mode (e.g., Mode::Mode0).
    pub fn begin(&mut self, bus: Bus, slave_select: SlaveSelect, clock_speed: u32, mode: Mode) {
        self.spi_module = Spi::new(bus, slave_select, clock_speed, mode)
    }

    fn send_bytes(&self, bytes: &[u8], context: &str) {
        match self.spi_module.as_ref() {
            Some(spi) => {
                if let Err(e) = spi.write(bytes) {
                    error!("SPI write failed in {}: {:?}", context, e);
                } else {
                    info!("{} command sent: {:?}", context, bytes);
                }
            }
            None => error!("SPI module not initialized in {}", context),
        }
    } 

    /// Sets the display and backlight state.
    ///
    /// # Arguments
    /// * `set_display` - If true, turns the display on; otherwise, off.
    /// * `set_bckl` - If true, turns the backlight on; otherwise, off.
    pub fn display_set(&self, set_display: bool, set_bckl: bool) {
        let msg = match (set_display, set_bckl) {
            (false, false) => [ESC, BRACKET, b'0', DISP_EN_CMD],
            (true, false) => [ESC, BRACKET, b'1', DISP_EN_CMD],
            (false, true) => [ESC, BRACKET, b'2', DISP_EN_CMD],
            (true, true) => [ESC, BRACKET, b'3', DISP_EN_CMD],
        };
        self.send_bytes(&msg, "display_set");
    }

    /// Sets the cursor and blink mode.
    ///
    /// # Arguments
    /// * `set_cursor` - If true, shows the cursor; otherwise, hides it.
    /// * `set_blink` - If true, enables cursor blinking; otherwise, disables it.
    pub fn cursor_mode_set(&self, set_cursor: bool, set_blink: bool) {
        let msg = match (set_cursor, set_blink) {
            (false, _) => [ESC, BRACKET, b'0', CURSOR_MODE_CMD],
            (true, false) => [ESC, BRACKET, b'1', CURSOR_MODE_CMD],
            (true, true) => [ESC, BRACKET, b'2', CURSOR_MODE_CMD],
        };
        self.send_bytes(&msg, "cursor_mode_set");
    }

    /// Clears the display and returns the cursor home.
    pub fn display_clear(&self) {
        let disp_clr = &[ESC, BRACKET, DISP_CLR_CMD];
        self.send_bytes(disp_clr, "display_clear");
    }

    /// Writes a string at a specified position on the display.
    ///
    /// # Arguments
    /// * `idx_row` - The row index (0-2).
    /// * `idx_col` - The column index (0-39).
    /// * `str_ln` - The string to write.
    ///
    /// # Returns
    /// * Error code indicating success or argument errors.
    pub fn write_string_at_pos(&self, idx_row: u8, idx_col: u8, str_ln: &str) -> u8 {
        if idx_row > 2 || idx_row < 0 {
            return LCDS_ERR_ARG_ROW_RANGE;
        }
        if idx_col > 39 || idx_col < 0 {
            return LCDS_ERR_ARG_COL_RANGE;
        }
        let first_digit = idx_col % 10;
        let second_digit = idx_col / 10;
        let mut length = str_ln.len();
        let length_to_print = length + idx_col as usize;
        if length_to_print > 40 {
            length = 40 - idx_col as usize;
        }
        let string_to_send = [ESC, BRACKET, idx_row + b'0', b';', second_digit + b'0', first_digit + b'0', CURSOR_POS_CMD];
        self.send_bytes(&string_to_send, "write_string_at_pos: set pos");
        let bytes_to_send = str_ln.as_bytes();
        self.send_bytes(&bytes_to_send[..length], "write_string_at_pos: data");
        LCDS_ERR_SUCCESS
    }

    /// Scrolls the display left or right by a specified number of columns.
    ///
    /// # Arguments
    /// * `direction` - true for right, false for left.
    /// * `idx_col` - Number of columns to scroll (0-39).
    ///
    /// # Returns
    /// * Error code indicating success or argument errors.
    pub fn display_scroll(&self, direction: bool, idx_col: u8) -> u8 {
        if idx_col < 0 || idx_col > 39 {
            return LCDS_ERR_ARG_COL_RANGE;
        }
        let first_digit = idx_col % 10;
        let second_digit = idx_col / 10;
        let r_scroll = &[ESC, BRACKET, second_digit + b'0', first_digit + b'0', RSCROLL_CMD];
        let l_scroll = &[ESC, BRACKET, second_digit + b'0', first_digit + b'0', LSCROLL_CMD];
        self.display_mode(true);
        if direction {
            self.send_bytes(r_scroll, "right scroll");
        } else {
            self.send_bytes(l_scroll, "left scroll");
        }
        LCDS_ERR_SUCCESS
    }

    /// Saves the current cursor position.
    pub fn save_cursor(&self) {
        let save_cursor = &[ESC, BRACKET, '0', CURSOR_SAVE_CMD];
        self.send_bytes(save_cursor);
    }

    /// Restores the previously saved cursor position.
    pub fn restore_cursor(&self) {
        let rest_cursor = &[ESC, BRACKET, '0', CURSOR_RSTR_CMD];
        self.send_bytes(rest_cursor);
    }

    /// Sets the display mode to wrap at 16 or 40 characters.
    ///
    /// # Arguments
    /// * `char_number` - true for 16 chars, false for 40 chars.
    pub fn display_mode(&self, char_number: bool) {
        let disp_mode_16 = &[ESC, BRACKET, '0', DISP_MODE_CMD];
        let disp_mode_40 = &[ESC, BRACKET, '1', DISP_MODE_CMD];

        if(char_number) {
            self.send_bytes(disp_mode_16, "display mode 16");
        } else {
            self.send_bytes(disp_mode_40, "display mode 40");
        }
    }

    /// Erases characters from a line based on the erase parameter.
    ///
    /// # Arguments
    /// * `erase_param` - 0: from current position to end of line, 1: start of line to current position, 2: entire line.
    ///
    /// # Returns
    /// * Error code indicating success or argument errors.
    pub fn erase_in_line(&self, erase_param: u8) -> u8 {
        if erase_param < 0 || erase_param > 2 {
            return LCDS_ERR_ARG_ERASE_OPTIONS;
        }
        let erase_mode = &[ESC, BRACKET, erase_param + b'0', ERASE_INLINE_CMD];
        self.send_bytes(erase_mode, "erase mode");
        LCDS_ERR_SUCCESS
    }

    /// Erases a number of characters starting at the current cursor position.
    ///
    /// # Arguments
    /// * `chars_number` - Number of characters to erase.
    pub fn erase_chars(&self, chars_number: u8) {
        let erase_chars = &[ESC, BRACKET, chars_number + b'0', ERASE_FIELD_CMD];
        self.send_bytes(erase_chars, "erasing chars at cursor");
    }

    /// Resets (cycles power of) the LCDS device.
    pub fn reset(&self) {
        let reset = &[ESC, BRACKET, '0', RST_CMD];
        self.send_bytes(reset, "reset LCDS");
    }

    /// Saves the TWI address to EEPROM.
    ///
    /// # Arguments
    /// * `addr_eeprom` - The EEPROM address to save.
    pub fn save_twi_addr(&self, addr_eeprom: u8) {
        let save_addr = &[ESC, BRACKET, addr_eeprom + b'0', TWI_SAVE_ADDR_CMD];
        self.send_bytes(save_addr, "saving twi address");
    }

    /// Saves the baud rate value to EEPROM.
    ///
    /// # Arguments
    /// * `baud_rate` - The baud rate value.
    ///
    /// # Returns
    /// * Error code indicating success or argument errors.
    pub fn save_br(&self, baud_rate: u8) -> u8 {
        if baud_rate < 0 || baud_rate > 6 {
            return LCDS_ERR_ARG_BR_RANGE;
        }
        let save_br = &[ESC, BRACKET, baud_rate + b'0', BR_SAVE_CMD];
        self.send_bytes(save_br, "saving baud rate");
        LCDS_ERR_SUCCESS
    }

    /// Programs a character table into the LCD.
    ///
    /// # Arguments
    /// * `char_table` - The character table index.
    ///
    /// # Returns
    /// * Error code indicating success or argument errors.
    pub fn chars_to_lcd(&self, char_table: u8) -> u8 {
        if char_table < 0 || char_table > 3 {
            return LCDS_ERR_ARG_TABLE_RANGE;
        }
        let progr_table = &[ESC, BRACKET, char_table + b'0', PRG_CHAR_CMD];
        self.send_bytes(progr_table, "programming char table");
        LCDS_ERR_SUCCESS
    }

    /// Saves a RAM character table to EEPROM.
    ///
    /// # Arguments
    /// * `char_table` - The character table index.
    ///
    /// # Returns
    /// * Error code indicating success or argument errors.
    pub fn save_ram_to_eeprom(&self, char_table: u8) -> u8 {
        if char_table < 0 || char_table > 3 {
            return LCDS_ERR_ARG_TABLE_RANGE;
        }
        let progr_table = &[ESC, BRACKET, char_table + b'0', SAVE_RAM_TO_EEPROM_CMD];
        self.send_bytes(progr_table);
        LCDS_ERR_SUCCESS
    }

    /// Loads a character table from EEPROM into RAM.
    ///
    /// # Arguments
    /// * `char_table` - The character table index.
    ///
    /// # Returns
    /// * Error code indicating success or argument errors.
    pub fn ld_eeprom_to_ram(&self, char_table: u8) -> u8 {
        if char_table < 0 || char_table > 3 {
            return LCDS_ERR_ARG_TABLE_RANGE;
        }
        let ld_table = &[ESC, BRACKET, char_table + b'0', LD_EEPROM_TO_RAM_CMD];
        self.send_bytes(ld_table, "ld_eeprom_to_ram");
        LCDS_ERR_SUCCESS
    }

    /// Saves the communication mode to EEPROM.
    ///
    /// # Arguments
    /// * `comm_sel` - The communication mode selection parameter.
    ///
    /// # Returns
    /// * Error code indicating success or argument errors.
    pub fn save_comm_to_eeprom(&self, comm_sel: u8) -> u8 {
        if comm_sel > 2 {
            return LCDS_ERR_ARG_COMM_RANGE;
        }
        let cmd = &[ESC, BRACKET, comm_sel + b'0', COMM_MODE_SAVE_CMD];
        self.send_bytes(cmd, "save_comm_to_eeprom");
        LCDS_ERR_SUCCESS
    }

    /// Enables the write operation to EEPROM.
    pub fn eeprom_wr_en(&self) {
        let cmd = &[ESC, BRACKET, b'0', EEPROM_WR_EN_CMD];
        self.send_bytes(cmd, "eeprom_wr_en");
    }

    /// Saves the cursor mode into EEPROM.
    ///
    /// # Arguments
    /// * `mode_crs` - The cursor mode parameter (0: off, 1: on, 2: blink).
    ///
    /// # Returns
    /// * Error code indicating success or argument errors.
    pub fn save_cursor_to_eeprom(&self, mode_crs: u8) -> u8 {
        if mode_crs < 0 || mode_crs > 2 {
            return LCDS_ERR_ARG_CRS_RANGE;
        }
        let cmd = &[ESC, BRACKET, mode_crs + b'0', CURSOR_MODE_SAVE_CMD];
        self.send_bytes(cmd, "save_cursor_to_eeprom");
        LCDS_ERR_SUCCESS
    }

    /// Saves the display mode into EEPROM.
    ///
    /// # Arguments
    /// * `mode_disp` - The display mode parameter (0: 16 chars, 1: 40 chars).
    ///
    /// # Returns
    /// * Error code indicating success or argument errors.
    pub fn save_display_to_eeprom(&self, mode_disp: u8) -> u8 {
        if mode_disp < 0 || mode_disp > 1 {
            return LCDS_ERR_ARG_DSP_RANGE;
        }
        let cmd = &[ESC, BRACKET, mode_disp + b'0', DISP_MODE_SAVE_CMD];
        self.send_bytes(cmd, "save_display_to_eeprom");
        LCDS_ERR_SUCCESS
    }

    /// Defines a character in memory at a specified location.
    ///
    /// # Arguments
    /// * `str_user_def` - The user-defined character data (8 bytes, one per row).
    /// * `char_pos` - The position in memory (0-7).
    ///
    /// # Returns
    /// * Error code indicating success or argument errors.
    pub fn define_user_char(&self, str_user_def: &[u8], char_pos: u8) -> u8 {
        if char_pos > 7 || char_pos < 0 {
            return LCDS_ERR_ARG_POS_RANGE;
        }
        let mut cmd: Vec<u8> = Vec::with_capacity(MAX);
        cmd.push(ESC);
        cmd.push(BRACKET);
        cmd.push(0);
        self.build_user_def_char(str_user_def, &mut cmd);
        cmd.push(char_pos + b'0');
        cmd.push(DEF_CHAR_CMD);
        cmd.push(ESC);
        cmd.push(BRACKET);
        cmd.push(b'3');
        cmd.push(PRG_CHAR_CMD);
        self.send_bytes(&cmd, "define_user_char");
        LCDS_ERR_SUCCESS
    }

    /// Displays a user-defined character at the specified position.
    ///
    /// # Arguments
    /// * `char_pos` - Array of character positions.
    /// * `char_number` - Number of characters to display.
    /// * `idx_row` - Row index.
    /// * `idx_col` - Column index.
    ///
    /// # Returns
    /// * Error code indicating success or argument errors.
    pub fn disp_user_char(&self, char_pos: &[u8], char_number: u8, idx_row: u8, idx_col: u8) -> u8 {
        if idx_row > 2 {
            return LCDS_ERR_ARG_ROW_RANGE;
        }
        if idx_col > 39 {
            return LCDS_ERR_ARG_COL_RANGE;
        }
        self.set_pos(idx_row, idx_col);
        let to_send = &char_pos[..(char_number as usize).min(char_pos.len())];
        self.send_bytes(to_send, "disp_user_char");
        LCDS_ERR_SUCCESS
    }

    /// Sets the position of the cursor.
    ///
    /// # Arguments
    /// * `idx_row` - Row index.
    /// * `idx_col` - Column index.
    ///
    /// # Returns
    /// * Error code indicating success or argument errors.
    pub fn set_pos(&self, idx_row: u8, idx_col: u8) -> u8 {
        if idx_row < 0 || idx_row > 2 {
            return LCDS_ERR_ARG_ROW_RANGE;
        }
        if idx_col < 0 || idx_col > 39 {
            return LCDS_ERR_ARG_COL_RANGE;
        }
        let first_digit = idx_col % 10;
        let second_digit = idx_col / 10;
        let str_to_send = &[ESC, BRACKET, idx_row + b'0', ';', second_digit + b'0', first_digit + b'0', CURSOR_POS_CMD];
        self.send_bytes(str_to_send, "set_pos");
        LCDS_ERR_SUCCESS
    }

    /// Builds the array format to be sent to the LCD for a user-defined character.
    ///
    /// # Arguments
    /// * `str_user_def` - The user-defined character data (8 bytes, one per row).
    /// * `cmd_str` - The output command buffer (Vec<u8> or &mut Vec<u8> recommended).
    pub fn build_user_def_char(&self, str_user_def: &[u8], cmd_str: &mut Vec<u8>) {
        // Each byte is converted to a string like "0xNN;" and appended as ASCII bytes
        for &val in str_user_def.iter().take(8) {
            // Format as uppercase hex, always two digits
            let hex = format!("0x{:02X};", val);
            cmd_str.extend_from_slice(hex.as_bytes());
        }
    }

}
