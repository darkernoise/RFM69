/*
 * AeroRFSensor.cpp
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

#include "AeroRFSensor.h"

AeroRFSensor::AeroRFSensor(): AeroRFBase::AeroRFBase() {
	last_command = 0; //initialize
	this->_command_start_recived = false;
}

void AeroRFSensor::run_cycle() {
	AeroRFBase::run_cycle();
	this->check_for_command_packet();
	//Process commands that are meant to continue
	this->process_command(this->last_command);
}

bool AeroRFSensor::initialize(){
	AeroRFBase::initialize();
	radio.promiscuous(1);
	radio.setListenOnly();
	return true;
}

//Output debugging only if serial is enabled
void AeroRFSensor::print_debug(uint8_t tagId, int16_t rssi) {
	#ifdef SERIAL_EN
	char debug_buff[50];
	sprintf(debug_buff, "Tag: %d RSSI: %d", tagId, rssi);
	DEBUGln(debug_buff);
	#endif
}

void AeroRFSensor::print_packet(uint8_t tagId, int16_t rssi) {
	#ifdef SERIAL_EN
	SER_WRITE(DATA_PREFIX_CHAR);
	SER_WRITE(DATA_PREFIX_CHAR);
	SER_WRITE(tagId);
	SER_WRITE(((uint8_t)rssi & 0xFF));
	SER_WRITE((rssi >> 8));
	#endif
}

//Checks hardware serial to see if a request
//for a command packet has been received
//
//Command packets can be at most 2 bytes
void AeroRFSensor::check_for_command_packet() {
	uint8_t tmp = 0;
//	char tmp = 0;
	uint8_t buff[50];
//	char buff[50];
	int indx=0;
	while ((Serial.available() > 0) && (indx < 50)){
		tmp = Serial.read();
//		Serial.println(tmp);
		buff[indx] = tmp;
		delay(1);
		indx++;
	}
	//process the buffer
	for (int i=0; i<indx; i++){
		tmp = buff[i];
//		Serial.print("Command received:");
//		Serial.println(tmp);
		if (tmp == CMD_START){
			this->_command_start_recived = true;
//			Serial.println("Command started...");
		}
		else if (this->_command_start_recived){
			//Full command is now received, so process
			this->last_command = tmp;
			this->process_command(tmp);
			this->_command_start_recived = false;
		}
		else{
			this->_command_start_recived = false;
		}
	}
}

//Performs a read on the radio
void AeroRFSensor::check_radio() {
	if (radio.receiveDone()){
		this->print_packet(radio.SENDERID, radio.RSSI);
		#ifdef DEBUG_EN
			this->print_debug(radio.SENDERID, radio.RSSI);
		#endif
		#ifdef STATUS_LED
			this->blink(STATUS_LED);
		#endif
	}
}

//Sends a sensor identification over serial
//
//Identification packet is in the form:
// 0 - <begin response>
// 1 - <command id>
// 2 - <network id>
// 3 - <node id>
// 4-35 - <guid>
// 36-44<created on>
// 45-47 - <version>
// 48-56 - registered on
// 59-69 - registration key
// 70-80 - Serial #
// 81 - Response Terminator

void AeroRFSensor::send_identification() {

	SER_WRITE(CMD_RESPONSE_START);
	SER_WRITE(CMD_IDENTIFY);
	SER_WRITE(this->getNetworkId());
	SER_WRITE(this->getNodeId());
	this->write_bytes_hex(this->get_guid(), AY_GUID_SIZE);
	this->write_bytes(this->get_created_on(), AY_DATE_SIZE);
	this->write_bytes(this->get_fw_version(), AY_VERSION_SIZE);
	this->write_bytes(this->get_registered_on(), AY_DATE_SIZE);
	this->write_bytes(this->get_registeration_key(), AY_REG_KEY_SIZE);
	this->write_bytes(this->get_serial_number(), AY_SERIAL_SIZE);
	SER_WRITE(CMD_TERMINATOR);
}

/*
 * Process specific command.
 */
void AeroRFSensor::process_command(uint8_t cmd) {
	switch (cmd){
		case 0:
//			this->send_identification();
			break;
		case CMD_IDENTIFY:
			this->send_identification();
			this->last_command = 0; //do not repeat
			break;
		case CMD_PRINT_INFO:
			this->print_info();
			this->last_command = 0; //do not repeat
			break;
		case CMD_LISTEN_START:
			this->check_radio();
			break;
		case CMD_LISTEN_STOP:
			//stop all communication
			this->last_command = 0; //do not repeat
			break;
	}
}


