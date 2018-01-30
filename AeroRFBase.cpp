/*
 * AeroRFBase.cpp
 *
 *  Created on: Oct 28, 2017
 *      Author: Joel Blackthorne
 *
 *  AeroRFLib is a radio library designed for long-distance, high-frequency,
 *  distance measurement as part of the AeroTracker project.
 *
 *
 *  This module is based on the RFM69HW_HCW and RFM69HW_HW HopeRF transceivers
 *  operating at 915mhz and will only work with those specific modules.
 *
 *  Extends work Copyright Felix Rusu 2016, http://www.LowPowerLab.com/contact
 */
// **********************************************************************************
// License
// **********************************************************************************
// This program is free software; you can redistribute it
// and/or modify it under the terms of the GNU General
// Public License as published by the Free Software
// Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will
// be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE. See the GNU General Public
// License for more details.
//
// Licence can be viewed at
// http://www.gnu.org/licenses/gpl-3.0.txt
//
// Please maintain this license information along with authorship
// and copyright notices in any redistribution of this code
// **********************************************************************************

#include "AeroRFBase.h"
#include "SPI.h"           //included with Arduino IDE install (www.arduino.cc)
#include "EEPROM.h"

AeroRFBase::AeroRFBase() {
}

AeroRFBase::~AeroRFBase() {
	// TODO Auto-generated destructor stub
}

/*
* The NodeID is the unique identifier for each
* radio.  Sensors are kept in the lower range,
* from 1 to 30 (defined by NODEID_SENSOR_CUTT_OFF).
* All node ids > NODEID_SENSOR_CUTT_OFF are considered
* target tags.
*/
uint8_t AeroRFBase::getNodeId() {
	return this->_nodeId;
}

uint8_t AeroRFBase::getNetworkId() {
	return this->_networkId;
}

bool AeroRFBase::initialize() {
	this->init_chdate(this->_created_on);
	this->init_chdate(this->_registered_on);
	//Load the EEPROM to the object attributes
	this->load_eeprom();
	bool rval = true;
#ifdef STATUS_LED
	pinMode(STATUS_LED, OUTPUT);
	this->blink(STATUS_LED);
#endif


	//Initialize the radio
	radio.initialize(FREQUENCY,this->_nodeId,this->_networkId);
	radio.setHighPower(); //must include this only for RFM69HW/HCW!

	//Output debugging only if serial is enabled
	#ifdef DEBUG_EN
		char buff[50];
		sprintf(buff, "\nTransmitting at %d Mhz...", int(radio.getFrequency()/1000000));
		DEBUGln(buff);
	#endif
	#ifdef STATUS_LED
		this->blink(STATUS_LED);
	#endif
	return rval;
}

/*
 * Single run cycle to be executed in main loop.
 */
void AeroRFBase::run_cycle() {
}

/*
 * Blinks a LED pin.
 */
void AeroRFBase::blink(uint8_t pin) {
#ifdef STATUS_LED
	if (this->_blink_on){
		this->_blink_on = false;
		digitalWrite(pin,LOW);
	}
	else{
		this->_blink_on = true;
		digitalWrite(pin,HIGH);
	}
	//This delay can help with debugging
//	delay(BLINK_DELAY);
#endif
}

uint8_t* AeroRFBase::get_guid() {
	return this->_guid;
}

/*
 * Read eeprom and set properties.
 */
bool AeroRFBase::read_all_eeprom(AeroEEPROM *eeprom_val) {

	uint8_t ev=0;

	for (int i=EEPROM_ADDR_START; i< IE_SERIAL; i++){
		ev = EEPROM.read(i);

		if (i==IS_CHECK_BYTE){
			eeprom_val->check_byte = ev;
			if (ev == 0){
				//No eeprom set, so abort
				return false;
			}
		}
		else if (i==IS_NETWORK_ID)
			eeprom_val->network_id = ev;
		else if (i==IS_NODE_ID)
			eeprom_val->node_id = ev;
		else if ((i >= IS_GUID) && (i < IE_GUID))
			eeprom_val->guid[i-IS_GUID] = ev;
		else if ((i >= IS_CREATED_ON) && (i < IE_CREATED_ON))
			eeprom_val->created_on[i-IS_CREATED_ON] = ev;
		else if ((i >= IS_VERSION) && (i < IE_VERSION)){
			eeprom_val->fw_version[i-IS_VERSION] = ev;
		}
		else if ((i >= IS_REG_ON) && (i < IE_REG_ON))
			eeprom_val->registerd_on[i-IS_REG_ON] = ev;
		else if ((i >= IS_REG_KEY) && (i < IE_REG_KEY))
			eeprom_val->regisration_key[i-IS_REG_KEY] = ev;
		else if ((i >= IS_SERIAL) && (i < IE_SERIAL))
			eeprom_val->serial_num[i-IS_SERIAL] = ev;

//		DEBUG("Read eeprom addr: ");
//		DEBUG(i);
//		DEBUG(" Value: ");
//		DEBUGln(ev);

	}
}

/*
 * Prints information about the device in
 * human readable form (ascii).
 */
void AeroRFBase::print_info() {
	int i=0;
	Serial.println("");
	Serial.println("**********************");
	Serial.println(PROD_NAME);
//
//	char buff[50];
//	sprintf(buff, "Transmitting at: %d Mhz", int(radio.getFrequency()/1000000));
//	Serial.println(buff);

	Serial.print("Version: ");
	Serial.print((this->_fw_version[0]));
	Serial.print(".");
	Serial.print((this->_fw_version[1]));
	Serial.print(".");
	Serial.print((this->_fw_version[2]));
	Serial.println("");

	Serial.print("Created: ");
	char created_ascii[AY_DATE_SIZE];
	this->byte_array_to_ascii(this->_created_on, created_ascii, AY_DATE_SIZE);
	for (i=0; i<8; i++){
		Serial.print(created_ascii[i]);
	}
	Serial.println("");

	Serial.print("Network: ");
	Serial.println(this->_networkId);

	Serial.print("Node Id: ");
	Serial.println(this->_nodeId);

	Serial.print("Serial #: ");
	char ser_ascii[AY_SERIAL_SIZE];
	this->byte_array_to_ascii(this->get_serial_number(), ser_ascii, AY_SERIAL_SIZE);
	for (i=0; i<AY_SERIAL_SIZE; i++){
		Serial.print(ser_ascii[i]);
	}
	Serial.println("");

	Serial.print("GUID: ");
	this->print_guid(this->get_guid());

	Serial.print("Registered On: ");
	char reg_ascii[AY_DATE_SIZE];
	this->byte_array_to_ascii(this->_registered_on, reg_ascii, AY_DATE_SIZE);
	//Note: there is a defect in Serial.println such that it is not writing
	//the custom types correctly.
	for (i=0; i<8; i++){
		Serial.print(reg_ascii[i]);
	}
	Serial.println("");

	Serial.print("Registration Key: ");
	char reg_key_ascii[AY_REG_KEY_SIZE];
	this->byte_array_to_ascii(this->_registration_key, reg_key_ascii, AY_REG_KEY_SIZE);
	for (i=0; i<AY_REG_KEY_SIZE; i++){
		Serial.print(reg_key_ascii[i]);
	}
	Serial.println("");

//	Serial.print("Size: ");
//	Serial.println(IE_SERIAL);
}

/*
 * When flashing firmware, these values are set to indicate
 * starting information.
 *
 * Parameters:
 *   created_on : 8 byte char* in form "YYYYMMDD"
 *   fw_version " 5 byte char* in form "<major>.<minor>.<dot>"
 */
void AeroRFBase::set_firmware_info(char* created_on, char* fw_version,
		uint8_t networkId, uint8_t nodeId, bool force_new_guid,
		char* guid_str, char* serial_number) {

	AeroEEPROM tmpEEPROM;
	uint8_t wrk=0;
	chdate tmp_date;

	this->read_all_eeprom(&tmpEEPROM);

	//initialize unused
	this->init_chdate(tmpEEPROM.registerd_on);
	this->init_regkey(tmpEEPROM.regisration_key);

	DEBUGln("Preparing to set eeprom");

	//Set check byte to indicate values flashed
	wrk=CHECK_BYTE_INITIAL_FLASH;
	//this->write_eeprom_char(&wrk, EEPROM_ADDR_START, 1);
	//Default Network and Node id
	this->write_eeprom_char(&networkId, IS_NETWORK_ID, 1);
	this->write_eeprom_char(&nodeId, IS_NODE_ID, 1);
	//created on date
	this->ascii_array_to_byte(created_on, tmp_date, AY_DATE_SIZE);
	this->write_eeprom_char(tmp_date, IS_CREATED_ON, AY_DATE_SIZE);
	//version
	this->version_str_to_bytes(fw_version, this->_fw_version);
	this->write_eeprom_char(this->_fw_version, IS_VERSION, AY_VERSION_SIZE);
	//GUID if needed
	if (force_new_guid) {
		this->guid_str_to_bytes(guid_str, tmpEEPROM.guid);
		this->write_eeprom_char(tmpEEPROM.guid, IS_GUID, AY_GUID_SIZE);

		//Serial Number
		AeroRFSerial ser;
		this->ascii_array_to_byte(serial_number, ser, AY_SERIAL_SIZE);
		this->write_eeprom_char(ser, IS_SERIAL, AY_SERIAL_SIZE);
	}
	//Registered on
	this->write_eeprom_char(tmpEEPROM.registerd_on, IS_REG_ON, AY_DATE_SIZE);
	//Registration key
	this->write_eeprom_char(tmpEEPROM.regisration_key, IS_REG_KEY, AY_REG_KEY_SIZE);

	DEBUGln("Completed setting eeprom");
	this->load_eeprom();
	this->print_info();
}

/*
 * Initializes a ch date value.
 */
void AeroRFBase::init_chdate(uint8_t* val) {
	for (int i=0; i<AY_DATE_SIZE; i++){
		val[i] = 0;
	}
}

/*
 * Reads an eeprom range to a char.
 */
bool AeroRFBase::read_eeprom_char(unsigned char *val, int addr, int len) {
	uint8_t tmp = 0;
	for (int i=0; i<len; i++){
		tmp = EEPROM.read(addr + i);
		val[addr + i] = tmp;
		DEBUG("Reading eeprom addr: ");
		DEBUG(addr + i);
		DEBUG(" value: ");
		DEBUGln(tmp);
	}
	return true;
}

/*
 * Loads current eeprom to object.
 */
void AeroRFBase::load_eeprom() {
	AeroEEPROM tmpEEPROM;
	this->read_all_eeprom(&tmpEEPROM);

	if (tmpEEPROM.check_byte > 0){
		this->_check_byte = tmpEEPROM.check_byte;
		this->_networkId = tmpEEPROM.network_id;
		this->_nodeId = tmpEEPROM.node_id;
		this->byte_array_copy(tmpEEPROM.guid, this->_guid, AY_GUID_SIZE);
		this->byte_array_copy(tmpEEPROM.created_on, this->_created_on, AY_DATE_SIZE);
		this->byte_array_copy(tmpEEPROM.fw_version, this->_fw_version, AY_VERSION_SIZE);
		this->byte_array_copy(tmpEEPROM.registerd_on, this->_registered_on, AY_DATE_SIZE);
		this->byte_array_copy(tmpEEPROM.regisration_key, this->_registration_key, AY_REG_KEY_SIZE);
		this->byte_array_copy(tmpEEPROM.serial_num, this->_serial_num, AY_SERIAL_SIZE);
	}
}

/*
 * Writes a variable to the eeprom.
 */
void AeroRFBase::write_eeprom_char(unsigned char* val, int addr, int len) {
	uint8_t tmp = 0;
	for (int i=0; i<len; i++){
		tmp = val[i];
		DEBUG("Writing eeprom addr: ");
		DEBUG(addr + i);
		DEBUG(" value: ");
		DEBUGln(tmp);
		EEPROM.write(addr + i, tmp);
	}
}

/*
 * Converts an ascii char list to byte list.
 */
void AeroRFBase::ascii_array_to_byte(char* ascii_lst, uint8_t* byte_lst, uint16_t lst_size) {
	for (uint16_t i=0; i< lst_size; i++){
		byte_lst[i] = (ascii_lst[i] - 48);
	}
}

/*
 * Converts byte list to printable ascii.
 */
void AeroRFBase::byte_array_to_ascii(uint8_t* byte_lst, char* ascii_lst,
		uint16_t lst_size) {
	for (uint16_t i=0; i< lst_size; i++){
		ascii_lst[i] = ((byte_lst[i]) + 48);
	}
}

/*
 * Copy byte array.
 */
void AeroRFBase::byte_array_copy(uint8_t* source_list, uint8_t* target_list,
		uint16_t lst_size) {
	for (uint16_t i=0; i< lst_size; i++){
		target_list[i] = source_list[i];
	}
}

/*
 * Converts a version string (5 byte char list) from the
 * form "<major>.<minor>.<dot>" into AeroRFVersion format.
 */
void AeroRFBase::version_str_to_bytes(char* version_list,
		AeroRFVersion ver_bytes) {
	ver_bytes[0] = (version_list[0]) - 48;
	ver_bytes[1] = (version_list[2]) - 48;
	ver_bytes[2] = (version_list[4]) - 48;
}

uint8_t* AeroRFBase::get_fw_version() {
	return this->_fw_version;
}

uint8_t* AeroRFBase::get_created_on() {
	return this->_created_on;
}

/*
 * Converts a GUID string to internal bytes.
 */
void AeroRFBase::guid_str_to_bytes(char* guid_str, AeroRFGUID guid) {
	int indx=0;
	int i=0;

	for (i=0; i<8; i++){
		guid[indx] =  this->ascii_char_to_hex(guid_str[i]);
		indx++;
	}
	for (i=9; i<13; i++){
		guid[indx] =  this->ascii_char_to_hex(guid_str[i]);
		indx++;
		}
	for (i=14; i<18; i++){
		guid[indx] =  this->ascii_char_to_hex(guid_str[i]);
		indx++;
	}
	for (i=19; i<23; i++){
		guid[indx] =  this->ascii_char_to_hex(guid_str[i]);
		indx++;
	}
	for (i=24; i<(AY_GUID_SIZE + 4); i++){
		guid[indx] =  this->ascii_char_to_hex(guid_str[i]);
		indx++;
	}
//	Serial.print(" Guid Str: ");
//	Serial.print(guid_str);
//	Serial.print(" = ");
//	this->print_guid(guid);
}

/*
 * Prints a guid in human readable form.
 */
void AeroRFBase::print_guid(AeroRFGUID guid) {
	char tmp_char = '0';
	for (int i=0; i<AY_GUID_SIZE; i++){
		tmp_char = this->hex_to_ascii_char(guid[i]);
		Serial.print(tmp_char);
		if (i == 7)
			Serial.print('-');
		else if (i ==11)
			Serial.print('-');
		else if (i ==15)
					Serial.print('-');
		else if (i ==19)
					Serial.print('-');
	}
	Serial.println("");
}

/*
 * Converts a hex byte to a readable char.
 */
char AeroRFBase::hex_to_ascii_char(uint8_t byte_val) {
	char rval = '0';
	if ((byte_val >= 0) && (byte_val <= 9)){
		rval = byte_val + 48;
	}
	else if(byte_val == 10)
		rval = 'A';
	else if(byte_val == 11)
		rval = 'B';
	else if(byte_val == 12)
		rval = 'C';
	else if(byte_val == 13)
		rval = 'D';
	else if(byte_val == 14)
		rval = 'E';
	else if(byte_val == 15)
		rval = 'F';
	return rval;
}

/*
 * Blanks and initializes the registration key.
 */
void AeroRFBase::init_regkey(AeroRFRegKey reg_key) {
	for (int i=0; i<AY_REG_KEY_SIZE; i++){
		reg_key[i] = 0;
	}
}

/*
 * Converts an ascii code to a hex byte value.
 */
uint8_t AeroRFBase::ascii_char_to_hex(char ascii_char) {
	uint8_t rval = 0;
	if ((ascii_char >= 48) && (ascii_char <= 57)){
		//numeric
		rval = ascii_char - 48;
	}
	else if((ascii_char >= 65) && (ascii_char <= 70)){
		rval = ascii_char - 55;
	}
	else if((ascii_char >= 97) && (ascii_char <= 102)){
		rval = ascii_char - 87;
	}
	return rval;
}

uint8_t* AeroRFBase::get_serial_number() {
	return this->_serial_num;
}

uint8_t* AeroRFBase::get_registered_on() {
	return this->_registered_on;
}

uint8_t* AeroRFBase::get_registeration_key() {
	return this->_registration_key;
}

void AeroRFBase::write_bytes_hex(uint8_t* lst_val, uint16_t size) {
	char tmp='0';
	for (uint16_t i=0; i<size; i++){
		tmp = this->hex_to_ascii_char(lst_val[i]);
		SER_WRITE(tmp);
	}
}

void AeroRFBase::write_bytes(uint8_t* lst_val, uint16_t size) {
	uint8_t tmp = 0;
	for (uint16_t i=0; i<size; i++){
		tmp = lst_val[i];
		SER_WRITE(tmp);
	}
}

void AeroRFBase::write_bytes_ascii(uint8_t* lst_val, uint16_t size) {
	char tmp='0';
	for (uint16_t i=0; i<size; i++){
		tmp = this->hex_to_ascii_char(lst_val[i]);
		SER_WRITE(tmp);
	}
}
