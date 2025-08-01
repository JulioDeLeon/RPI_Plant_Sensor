/************************************************************************/
/*																		*/
/*	LCDS.cpp	--	Definition for LCDS library 	    				*/
/*																		*/
/************************************************************************/
/*	Author:		Monica Bot												*/
/*	Copyright 2011, Digilent Inc.										*/
/************************************************************************/
/*  File Description:													*/
/*		This file defines functions for LCDS							*/
/*																		*/
/************************************************************************/
/*  Revision History:													*/
/*																		*/
/*	10/03/2011(MonicaB): created										*/
/*																		*/
/************************************************************************/


/* ------------------------------------------------------------ */
/*				Include File Definitions						*/
/* ------------------------------------------------------------ */
#include <LCDS.h>
#include "DSPI.h"

/* ------------------------------------------------------------ */
/*				Procedure Definitions							*/
/* ------------------------------------------------------------ */
/* ------------------------------------------------------------ */
/*        LCDS::LCDS
**
**        Synopsis:
**				
**        Parameters:
**
**
**
**        Return Values:
**                void 
**
**        Errors:
**
**
**        Description:
**			Class constructor. Performs variables initialization tasks
**
**
*/
LCDS::LCDS()
{
	pdspi = NULL;
}
/* ------------------------------------------------------------------- */
/** void LCDS::Begin(uint8_t accessType)
**
**	Parameters:
**		accessType - the SPI port chosen for the communication
**		
**
**	Return Value:
**		None
**
**	Errors:
**		none
**
**	Description:
**		This function initializes and configures the SPI interface 
**
-----------------------------------------------------------------------*/
 void LCDS::Begin(uint8_t accessType) {
	// declare the SPI port to be used
	m_accessType = accessType;
	if(m_accessType == PAR_ACCESS_DSPI0) {
		pdspi = new DSPI0();
		m_SSPin = PIN_DSPI0_SS;
	}
	if(m_accessType == PAR_ACCESS_DSPI1) {
		pdspi = new DSPI1();
		m_SSPin = PIN_DSPI1_SS;
	}
	// init SPI  
	pdspi->setPinSelect(m_SSPin);	
	pdspi->begin();
	pdspi->setSpeed(PAR_SPD_MAX);
	pdspi->setMode(DSPI_MODE0);
}
/* ------------------------------------------------------------------- */
/** void LCDS::SendBytes(unsigned char* dataChars, int lenght)
**
**	Parameters:
**		dataChars - the string to be sent to SPI
**		lenght - the lenght of the string to be sent
**		
**
**	Return Value:
**		None
**
**	Errors:
**		none
**
**	Description:
**		This function sends an array of characters to the SPI interface using the Arduino standard functions
**
-----------------------------------------------------------------------*/
void LCDS :: SendBytes(unsigned char* dataChars, uint8_t lenght){
	if (pdspi != NULL) {
		digitalWrite(m_SSPin, LOW);
		for(int i = 0; i < lenght; i++) {
			pdspi->transfer(dataChars[i]);
		}
		digitalWrite(m_SSPin, HIGH);
	}
}
/* ------------------------------------------------------------------- */
/** void LCDS::DisplaySet(bool setDisplay, bool setBckl)
**
**	Parameters:
**		setDisplay - boolean parameter through which the display is set on or off
**		setBckl - boolean parameter through which the backlight is set on or off
**
**
**	Return Value:
**		None
**
**	Errors:
**		none
**
**	Description:
**		This function turns the display and the backlight on or off, according to the user's selection
**
-----------------------------------------------------------------------*/
void LCDS::DisplaySet(boolean setDisplay, boolean setBckl) {
    uint8_t dispBcklOff []  = {ESC, BRACKET, '0', DISP_EN_CMD};
	uint8_t dispOnBckl  []  = {ESC, BRACKET, '1', DISP_EN_CMD};
	uint8_t dispBcklOn  []  = {ESC, BRACKET, '2', DISP_EN_CMD};
	uint8_t dispOnBcklOn[]  = {ESC, BRACKET, '3', DISP_EN_CMD};
	
	if ((!setDisplay)&&(!setBckl))	{
		//send the command for both display and backlight off
		SendBytes(dispBcklOff, 4);
	}
	else if ((setDisplay)&&(!setBckl))	{
		//send the command for display on and backlight off
		SendBytes(dispOnBckl, 4);
		}
		else if ((!setDisplay)&&(setBckl))	{
			//send the command for backlight on and display off
			SendBytes(dispBcklOn, 4);
		}
			else {
				//send the command for both display and backlight on
				SendBytes(dispOnBcklOn, 4);
			}
}
/* ------------------------------------------------------------------- */
/** void LCDS::CursorModeSet(bool setCursor, bool setBlink)
**
**	Parameters:
**		setCursor - boolean parameter through which the cursor is set on or off
**		setBlink - boolean parameter through which the blink option is set on or off
**
**
**	Return Value:
**		None
**
**	Errors:
**		none
**
**	Description:
**		This function turns the cursor and the blinking option on or off, according to the user's selection
**
-----------------------------------------------------------------------*/
void LCDS::CursorModeSet(boolean setCursor, boolean setBlink) {
	uint8_t cursorOff[]  		  = {ESC, BRACKET, '0', CURSOR_MODE_CMD};
	uint8_t cursorOnBlinkOff[]    = {ESC, BRACKET, '1', CURSOR_MODE_CMD};
	uint8_t cursorBlinkOn[]       = {ESC, BRACKET, '2', CURSOR_MODE_CMD};
	if (!setCursor)	{
		//send the command for both display and blink off
		SendBytes(cursorOff, 4);
	}
	else if ((setCursor)&&(!setBlink)) {
		//send the command for display on and blink off
		SendBytes(cursorOnBlinkOff, 4);
	}
		else {
			//send the command for display and blink on
			SendBytes(cursorBlinkOn, 4);
		}
}

/* ------------------------------------------------------------------- */
/** void LCDS::DisplayClear()
**
**	Parameters:
**		none
**
**	Return Value:
**		none
**
**	Errors:
**		none
**
**	Description:
**		This function clears the display and returns the cursor home
**
-----------------------------------------------------------------------*/
void LCDS::DisplayClear() {
	uint8_t dispClr[] = {ESC, BRACKET, DISP_CLR_CMD};
	//clear the display and returns the cursor home
	SendBytes(dispClr, 3);
}

/* ------------------------------------------------------------------- */
/** uint8_t  LCDS::WriteStringAtPos(uint8_t idxLine, uint8_t idxPos, char* strLn)
**
**	Parameters:
**		idxLine - the line where the string is written: 0 or 1
**		idxPos  - the start column for the string to be written:0 to 39
**		strLn   - the string to be written
**
**	Return Value:
**		uint8_t 
**					- LCDS_ERR_SUCCESS - The action completed successfully
**					- a combination of the following errors(OR-ed): 
**						- LCDS_ERR_ARG_COL_RANGE - The argument is not within 0, 39 range
**						- LCDS_ERR_ARG_ROW_RANGE - The argument is not within 0, 2 range
**
**	Errors:
**		see returned values
**
**	Description:
**		This function writes a string at a specified position	
**
-----------------------------------------------------------------------*/
uint8_t LCDS::WriteStringAtPos(uint8_t idxRow, uint8_t idxCol, char* strLn) {
	int i;
	int j;
	uint8_t bResult = LCDS_ERR_SUCCESS;
	if (idxRow < 0 || idxRow > 2){
		bResult |= LCDS_ERR_ARG_ROW_RANGE;
	}
	if (idxCol < 0 || idxCol > 39){
		bResult |= LCDS_ERR_ARG_COL_RANGE;
	}
	if (bResult == LCDS_ERR_SUCCESS){
		//separate the position digits in order to send them, useful when the position is greater than 10
		uint8_t firstDigit 		= idxCol % 10;
		uint8_t secondDigit 	= idxCol / 10;
		uint8_t length 			= strlen(strLn);
		uint8_t lengthToPrint   = length + idxCol;
		uint8_t stringToSend[]  = {ESC, BRACKET, idxRow + '0', ';', secondDigit + '0', firstDigit + '0', CURSOR_POS_CMD};
		if (lengthToPrint > 40) {
			//truncate the lenght of the string 
			//if it's greater than the positions number of a line
			length = 40 - idxCol;
		}
		SendBytes(stringToSend, 7);
		SendBytes((byte *)strLn, length);
	}
	return bResult;
}
/* ------------------------------------------------------------------- */
/** uint8_t  LCDS::DisplayScroll(boolean fDirection, uint8_t idxCol)
**
**	Parameters:
**		Direction - true for scroll right, false for left
**		idxCol	- parameter indicating the number of columns to scroll the text
**
**	Return Value:
**		uint8_t 
**					- LCDS_ERR_SUCCESS - The action completed successfully 
**					- LCDS_ERR_ARG_COL_RANGE - The argument is not within 0, 39 range
**
**	Errors:
**		none
**
**	Description:
**		This function scrolls left or right the display	with a specified number of columns
**
-----------------------------------------------------------------------*/
uint8_t LCDS::DisplayScroll(boolean fDirection, uint8_t idxCol) {
	uint8_t bResult;
	if (idxCol >= 0 && idxCol <= 39){
		//separate the position digits in order to send them, useful when the position is greater than 10
		uint8_t firstDigit 		= idxCol % 10;
		uint8_t secondDigit 	= idxCol / 10;
		uint8_t rScroll[]     	= {ESC, BRACKET, secondDigit + '0', firstDigit + '0', RSCROLL_CMD};
		uint8_t lScroll[]       = {ESC, BRACKET, secondDigit + '0', firstDigit + '0', LSCROLL_CMD};
		DisplayMode(true);
		if (fDirection) {
			//scroll right with idxCol columns
			SendBytes(rScroll, 5);
		}
		else {
			//scroll left with idxCol columns
			SendBytes(lScroll, 5);
		}
		bResult = LCDS_ERR_SUCCESS;
	}
	else {
		bResult = LCDS_ERR_ARG_COL_RANGE;
	}
	return bResult;
 }
/* ------------------------------------------------------------------- */
/** void  LCDS::SaveCursor()
**
**	Parameters:
**		none
**		
**
**	Return Value:
**		none
**
**	Errors:
**		none
**
**	Description:
**		This function saves the current position of the cursor
**
-----------------------------------------------------------------------*/
void LCDS::SaveCursor(){
	uint8_t saveCursor[] = {ESC, BRACKET, '0', CURSOR_SAVE_CMD};
	//send the save cursor position command
	SendBytes(saveCursor, 4);
}
/* ------------------------------------------------------------------- */
/** void  LCDS::RestoreCursor()
**
**	Parameters:
**		none
**		
**
**	Return Value:
**		none
**
**	Errors:
**		none
**
**	Description:
**		This function restores the previously saved position of the cursor
**
-----------------------------------------------------------------------*/
void LCDS::RestoreCursor(){
	uint8_t restCursor[] = {ESC, BRACKET, '0', CURSOR_RSTR_CMD};
	//send the restore cursor position command
	SendBytes(restCursor, 4);
}

/* ------------------------------------------------------------------- */
/** void  LCDS::DisplayMode(boolean charNumber)
**
**	Parameters:
**		charNumber - parameter for selecting the wrapping type of the line: to 16 or 40 characters
**		
**
**	Return Value:
**		none
**
**	Errors:
**		none
**
**	Description:
**		This function wraps the line at 16 or 40 characters
**
-----------------------------------------------------------------------*/
void LCDS::DisplayMode(boolean charNumber){
	uint8_t dispMode16[] = {ESC, BRACKET, '0', DISP_MODE_CMD};
	uint8_t dispMode40[] = {ESC, BRACKET, '1', DISP_MODE_CMD};
	if (charNumber){
		//wrap line at 16 characters
		SendBytes(dispMode16, 4);
	}
	else{
		//wrap line at 40 characters
		SendBytes(dispMode40, 4);
	}
}
/* ------------------------------------------------------------------- */
/** uint8_t  LCDS::EraseInLine(uint8_t eraseParam)
**
**	Parameters:
**		eraseParam - the selection of erasing operation: from current position, entire line, from the beginning of the line
**				   0 = current position to end of line
**		           1 = start of line to current position
**				   2 = entire line
**
**	Return Value:
**		uint8_t 
**					- LCDS_ERR_SUCCESS - The action completed successfully 
**					- LCDS_ERR_ARG_ERASE_OPTIONS - The argument is not within 0, 2 range
**
**	Errors:
**		none
**
**	Description:
**		This function erases characters from a line
**
-----------------------------------------------------------------------*/
uint8_t LCDS::EraseInLine(uint8_t eraseParam){
	uint8_t bResult;
	if (eraseParam >= 0 && eraseParam <= 2){
		uint8_t eraseMode[] = {ESC, BRACKET, (char)eraseParam + '0', ERASE_INLINE_CMD};
		//send command for erasing characters according to the eraseParam
		SendBytes(eraseMode, 4);
		bResult = LCDS_ERR_SUCCESS;
	}
	else {
		bResult = LCDS_ERR_ARG_ERASE_OPTIONS;
	}
	return bResult;
}
/* ------------------------------------------------------------------- */
/** void  LCDS::EraseChars(uint8_t charsNumber)
**
**	Parameters:
**		charsNumber - the number of characters to be erased
**		
**
**	Return Value:
**		none
**
**	Errors:
**		none
**
**	Description:
**		This function erases a number of characters starting at the current position
**
-----------------------------------------------------------------------*/
void LCDS::EraseChars(uint8_t charsNumber){
	uint8_t eraseChars[] = {ESC, BRACKET, (char)charsNumber + '0', ERASE_FIELD_CMD};
	SendBytes(eraseChars, 4);
}
/* ------------------------------------------------------------------- */
/** void  LCDS::Reset()
**
**	Parameters:
**		none
**		
**
**	Return Value:
**		none
**
**	Errors:
**		none
**
**	Description:
**		This function cycles power of LCDS
**
-----------------------------------------------------------------------*/
void LCDS::Reset(){
	uint8_t reset[] = {ESC, BRACKET, '0', RST_CMD};
	SendBytes(reset, 4);
}
/* ------------------------------------------------------------------- */
/** void  LCDS::SaveTWIAddr(uint8_t addrEeprom)
**
**	Parameters:
**		addrEeprom - the location where to save the TWI address
**		
**
**	Return Value:
**		none
**
**	Errors:
**		none
**
**	Description:
**		This function saves a TWI address to an EEPROM memory location
**
-----------------------------------------------------------------------*/
void LCDS::SaveTWIAddr(uint8_t addrEeprom){
	uint8_t saveAddr[] = {ESC, BRACKET, addrEeprom + '0', TWI_SAVE_ADDR_CMD};
	SendBytes(saveAddr, 4);
}

/* ------------------------------------------------------------------- */
/** uint8_t  LCDS::SaveBR(uint8_t baudRate)
**
**	Parameters:
**		baudRate - the baud rate value to be saved in the EEPROM memory
**		
**
**	Return Value:
**		uint8_t 
**					- LCDS_ERR_SUCCESS - The action completed successfully 
**					- LCDS_ERR_ARG_BR_RANGE - The argument is not within 0, 6 range
**
**	Errors:
**		none
**
**	Description:
**		This function saves the baud rate value to an EEPROM memory location
**
-----------------------------------------------------------------------*/
uint8_t LCDS::SaveBR(uint8_t baudRate){
		/*EEPROM based baud rate for USART
	allowed values:
	0 -> 2400
	1 -> 4800
	2 -> 9600
	3 -> 19200
	4 -> 38400
	5 -> 76800
	*/
	uint8_t bResult;
	if (baudRate >= 0 && baudRate <= 6){
		uint8_t saveBR[] = {ESC, BRACKET, baudRate + '0', BR_SAVE_CMD};
		SendBytes(saveBR, 4);
		bResult = LCDS_ERR_SUCCESS;
	}
	else {
		bResult = LCDS_ERR_ARG_BR_RANGE;
	}
	return bResult;
}
/* ------------------------------------------------------------------- */
/** uint8_t  LCDS::CharsToLcd(uint8_t charTable)
**
**	Parameters:
**		charTable - the characters table index to program into LCD - in EEPROM or RAM
**		
**
**	Return Value:
**		uint8_t 
**					- LCDS_ERR_SUCCESS - The action completed successfully 
**					- LCDS_ERR_ARG_TABLE_RANGE - The argument is not within 0, 3 range
**
**	Errors:
**		none
**
**	Description:
**		This function programs a table from EEPROM or RAM into LCD
**
-----------------------------------------------------------------------*/
uint8_t LCDS::CharsToLcd(uint8_t charTable){
	uint8_t bResult;
	if (charTable >= 0 && charTable <= 3){
		uint8_t progrTable[] = {ESC, BRACKET, charTable + '0', PRG_CHAR_CMD};
		SendBytes(progrTable, 4);
		bResult = LCDS_ERR_SUCCESS;
	}
	else {
		bResult = LCDS_ERR_ARG_TABLE_RANGE;
	}
	return bResult;
}
/* ------------------------------------------------------------------- */
/** uint8_t  LCDS::SaveRamtoEeprom(uint8_t charTable)
**
**	Parameters:
**		charTable - the characters table index to save in EEPROM from RAM
**		
**
**	Return Value:
**		uint8_t 
**					- LCDS_ERR_SUCCESS - The action completed successfully 
**					- LCDS_ERR_ARG_TABLE_RANGE - The argument is not within 0, 3 range
**
**	Errors:
**		none
**
**	Description:
**		This function saves a table in EEPROM from RAM
**
-----------------------------------------------------------------------*/
uint8_t LCDS::SaveRamtoEeprom(uint8_t charTable){
	uint8_t bResult;
	if (charTable >= 0 && charTable <= 3){
		uint8_t progrTable[] = {ESC, BRACKET, charTable + '0', SAVE_RAM_TO_EEPROM_CMD};
		SendBytes(progrTable, 4);
		bResult = LCDS_ERR_SUCCESS;
	}
	else {
		bResult = LCDS_ERR_ARG_TABLE_RANGE;
	}
	return bResult;
}

/* ------------------------------------------------------------------- */
/** uint8_t  LCDS::LdEepromToRam(uint8_t charTable)
**
**	Parameters:
**		charTable - the characters table index to save in RAM from EEPROM  
**		
**
**	Return Value:
**		uint8_t 
**					- LCDS_ERR_SUCCESS - The action completed successfully 
**					- LCDS_ERR_ARG_TABLE_RANGE - The argument is not within 0, 3 range
**
**	Errors:
**		none
**
**	Description:
**		This function loads a table in RAM from EEPROM  
**
-----------------------------------------------------------------------*/
uint8_t LCDS::LdEepromToRam(uint8_t charTable){
	uint8_t bResult;
	if (charTable >= 0 && charTable <= 3){
		uint8_t ldTable[] = {ESC, BRACKET, charTable + '0', LD_EEPROM_TO_RAM_CMD};
		SendBytes(ldTable, 4);
		bResult = LCDS_ERR_SUCCESS;
	}
	else {
		bResult = LCDS_ERR_ARG_TABLE_RANGE;
	}
	return bResult;
}

/* ------------------------------------------------------------------- */
/** uint8_t  LCDS::SaveCommToEeprom(uint8_t commSel)
**
**	Parameters:
**		commSel - communication mode selection parameter 
**		
**
**	Return Value:
**		uint8_t 
**					- LCDS_ERR_SUCCESS - The action completed successfully 
**					- LCDS_ERR_ARG_COMM_RANGE - The argument is not within 0, 7 range
**
**	Errors:
**		none
**
**	Description:
**		This function saves the communication mode to EEPROM  
**
-----------------------------------------------------------------------*/
uint8_t LCDS::SaveCommToEeprom(uint8_t commSel){
	uint8_t bResult;
/*	MD2, MD1, MD0 		Protocol 			Details
	0,0,0 				UART 				2400 baud
	0,0,1 				UART 				4800 baud
	0,1,0 				UART 				9600 baud
	0,1,1 				UART 				baud rate in EEPROM
	1,0,0 				TWI 				address: 0x48
	1,0,1 				TWI 				address in EEPROM
	1,1,0 				SPI
	1,1,1 				specified in EEPROM specified in EEPROM
*/
	if (commSel >= 0 && commSel <= 7){
		uint8_t commMode[] = {ESC, BRACKET, commSel + '0', COMM_MODE_SAVE_CMD};
		SendBytes(commMode, 4);
		bResult = LCDS_ERR_SUCCESS;
	}
	else {
		bResult = LCDS_ERR_ARG_COMM_RANGE;
	}
	return bResult;
}

/* ------------------------------------------------------------------- */
/** void  LCDS::EepromWrEn()
**
**	Parameters:
**		none 
**		
**
**	Return Value:
**		none
**
**	Errors:
**		none
**
**	Description:
**		This function enables the write operation to EEPROM
**
-----------------------------------------------------------------------*/
void LCDS::EepromWrEn(){
	uint8_t wrEn[] = {ESC, BRACKET,'0', EEPROM_WR_EN_CMD};
	SendBytes(wrEn, 4);
}
/* ------------------------------------------------------------------- */
/** uint8_t  LCDS::SaveCursorToEeprom(byte modeCrs)
**
**	Parameters:
**		modeCrs - parameter for selecting the mode of the cursor 
**		
**
**	Return Value:
**		uint8_t 
**					- LCDS_ERR_SUCCESS - The action completed successfully 
**					- LCDS_ERR_ARG_CRS_RANGE - The argument is not within 0, 2 range
**
**	Errors:
**		none
**
**	Description:
**		This function saves the cursor mode into EEPROM memory
**
-----------------------------------------------------------------------*/
uint8_t LCDS::SaveCursorToEeprom(uint8_t modeCrs){
	uint8_t bResult;
	if (modeCrs >= 0 && modeCrs <= 2){
		uint8_t crsSave[] = {ESC, BRACKET,modeCrs + '0', CURSOR_MODE_SAVE_CMD};
		SendBytes(crsSave, 4);
		bResult = LCDS_ERR_SUCCESS;
	}
	else {
		bResult = LCDS_ERR_ARG_CRS_RANGE;
	}
	return bResult;
}

/* ------------------------------------------------------------------- */
/** uint8_t  LCDS::SaveDisplayToEeprom(uint8_t modeDisp)
**
**	Parameters:
**		modeDisp - parameter for selecting the display mode 
**		
**
**	Return Value:
**		uint8_t 
**					- LCDS_ERR_SUCCESS - The action completed successfully 
**					- LCDS_ERR_ARG_DSP_RANGE - The argument is not within 0, 3 range
**
**	Errors:
**		none
**
**	Description:
**		This function saves the display mode into EEPROM memory
**
-----------------------------------------------------------------------*/
uint8_t LCDS::SaveDisplayToEeprom(uint8_t modeDisp){
	uint8_t bResult;
	if (modeDisp >= 0 && modeDisp <= 3){
		uint8_t dispSave[] = {ESC, BRACKET, modeDisp + '0', DISP_MODE_SAVE_CMD};
		SendBytes(dispSave, 4);
		bResult = LCDS_ERR_SUCCESS;
	}
	else {
		bResult = LCDS_ERR_ARG_DSP_RANGE;
	}
	return bResult;
}
/* ------------------------------------------------------------------- */
/** unint8_t  LCDS::SetPos(uint8_t idxRow, uint8_t idxCol)
**
**	Parameters:
**		idxRow - the row where the cursor will be positioned
**		idxCol - the column at which the cursor will be positioned
**		
**
**	Return Value:
**		uint8_t 
**					- LCDS_ERR_SUCCESS - The action completed successfully 
**					- a combination of the following errors (OR-ed):
**						- LCDS_ERR_ARG_COL_RANGE - The argument is not within 0, 39 range
**						- LCDS_ERR_ARG_ROW_RANGE - The argument is not within 0, 2 range
**
**	Errors:
**		none
**
**	Description:
**		This function sets the cursor position
**
-----------------------------------------------------------------------*/
uint8_t LCDS::SetPos(uint8_t idxRow, uint8_t idxCol) {
	uint8_t bResult = LCDS_ERR_SUCCESS;
	if (idxRow < 0 || idxRow > 2){
		bResult |= LCDS_ERR_ARG_ROW_RANGE;
	}
	if (idxCol < 0 || idxCol > 39){
		bResult |= LCDS_ERR_ARG_COL_RANGE;
	}
	if (bResult == LCDS_ERR_SUCCESS){
		//separate the position digits in order to send them, useful when the position is greater than 10
		uint8_t firstDigit 	= idxCol % 10;
		uint8_t secondDigit 	= idxCol / 10;
		uint8_t stringToSend[] = {ESC, BRACKET, idxRow + '0', ';', secondDigit + '0', firstDigit + '0', CURSOR_POS_CMD};
		SendBytes(stringToSend, 7);
	}
	return	bResult;
}

/* ------------------------------------------------------------------- */
/** uint8_t  LCDS::DefineUserChar((uint8_t* strUserDef, uint8_t charPos)
**
**	Parameters:
**		strUserDef - characters array containing the numerical value of each row in the char
**		charPos - the position of the character saved in the memory 
**		
**
**	Return Value:
**		uint8_t 
**					- LCDS_ERR_SUCCESS - The action completed successfully 
**					- LCDS_ERR_ARG_POS_RANGE - The argument is not within 0, 7 range
**
**	Errors:
**		none
**
**	Description:
**		This function saves a user defined char in the RAM memory
**
-----------------------------------------------------------------------*/
uint8_t LCDS::DefineUserChar(uint8_t* strUserDef, uint8_t charPos) {
	char rgcCmd[MAX];
	uint8_t bResult;
	if (charPos >= 0 && charPos <= 7){
		rgcCmd[0] = ESC;
		rgcCmd[1] = BRACKET;
		rgcCmd[2] = 0;
		//build the values to be sent for defining the custom character
		BuildUserDefChar(strUserDef, rgcCmd + 2);
		byte bLength = strlen(rgcCmd);
		rgcCmd[bLength++] = (char)charPos + '0';
		rgcCmd[bLength++] = DEF_CHAR_CMD;
		//save the defined character in the RAM
		rgcCmd[bLength++] = ESC;
		rgcCmd[bLength++] = BRACKET;
		rgcCmd[bLength++] = '3';
		rgcCmd[bLength++] = PRG_CHAR_CMD;
		SendBytes((unsigned char*)(rgcCmd), bLength);
		bResult = LCDS_ERR_SUCCESS;
	}
	else {
		bResult = LCDS_ERR_ARG_POS_RANGE;
	}
	return bResult;
}
/* ------------------------------------------------------------------- */
/** void  LCDS::DispUserChar(uint8_t* charPos, uint8_t charNumber, uint8_t idxRow, uint8_t idxCol)
**
**	Parameters:
**		charPos - array containing the positions of the characters saved in the RAM memory 
**		charNumber - the number of custom chars to be displayed on the LCD
**		idxRow - the row at which the first character should be displayed
**		idxCol - the column starting from where the character should be displayed
**
**	Return Value:
**		uint8_t 
**					- LCDS_ERR_SUCCESS - The action completed successfully
**					- a combination of the following errors (OR-ed): 
**						- LCDS_ERR_ARG_COL_RANGE - The argument is not within 0, 39 range
**						- LCDS_ERR_ARG_ROW_RANGE - The argument is not within 0, 2 range
**
**	Errors:
**		none
**
**	Description:
**		This function displays a user defined char at the specified position on the LCD
**
-----------------------------------------------------------------------*/
uint8_t LCDS::DispUserChar(uint8_t* charPos, uint8_t charNumber, uint8_t idxRow, uint8_t idxCol) {
	uint8_t bResult = LCDS_ERR_SUCCESS;
	if (idxRow < 0 || idxRow > 2){
		bResult |= LCDS_ERR_ARG_ROW_RANGE;
	}
	if (idxCol < 0 || idxCol > 39){
		bResult |= LCDS_ERR_ARG_COL_RANGE;
	}
	if (bResult == LCDS_ERR_SUCCESS){
		//set the position of the cursor to the wanted line/column for displaying custom chars
		SetPos(idxRow, idxCol);
		//send the position of the character or characters that have to be displayed, and their number
		SendBytes(charPos, charNumber);
	}
	return bResult;
}
/* ------------------------------------------------------------------- */
/** void  LCDS::BuildUserDefChar(uint8_t* strUserDef,  char* cmdStr)
**
**	Parameters:
**		strUserDef - bytes array containing the values to be converted in values that are recognized by the firmware
**		cmdStr	   - characters array containing the values converted 
**		
**
**	Return Value:
**		none
**
**	Errors:
**		none
**
**	Description:
**		This function builds the string to be converted in an interpretable array of chars for the LCD
**
-----------------------------------------------------------------------*/
void LCDS::BuildUserDefChar(uint8_t* strUserDef, char* cmdStr) {
	uint8_t len = 8;
	char elStr[4];
	//print the bytes from the input array as hex values
	for(int i = 0; i < len; i++){
		sprintf(elStr, "%2.2X", strUserDef[i]);
		//concatenate the result with the 0x chars to be able to send it to the LCD
		strcat(cmdStr, "0x");
		//since the Arduino compiler has a bug in the sprintf function, so it doesn't interpret the 
		//precision and width specifiers as it should, by adding a 0 in front of a number greater than 16,
		//we need to eliminate that 0 from the string
		if (strUserDef[i] > 15) {
			elStr[3] = 0;
			strcat(cmdStr, elStr + 1);
		}
		else {
			elStr[2] = 0;
			strcat(cmdStr, elStr);
		}
		strcat(cmdStr, ";");
	}
}