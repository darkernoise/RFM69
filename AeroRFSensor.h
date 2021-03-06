/*
 * AeroRFSensor.h
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

//*********************************************************************************************
// Command Packet Definitions:
#define CMD_START 0x53 			//start of command
#define CMD_IDENTIFY 0x49		//request identification
#define CMD_PRINT_INFO 0x50		//print human readable information
#define CMD_LISTEN_START 0x68	//start listening to sensors
#define CMD_LISTEN_STOP 0x69 	//stop listening to sensors
#define CMD_RESPONSE_START 0xFC
#define CMD_TERMINATOR 0xB2
//*********************************************************************************************

#ifndef AERORFSENSOR_H_
#define AERORFSENSOR_H_

#include "AeroRFBase.h"

#define DATA_PREFIX_CHAR 0xFA

class AeroRFSensor: public AeroRFBase {
public:
	char last_command;
	AeroRFSensor();
	void run_cycle();
	bool initialize();
private:
	bool _command_start_recived;
	void print_debug(uint8_t tagId, int16_t rssi);
	void print_packet(uint8_t tagId, int16_t rssi);
	void check_radio();
	void check_for_command_packet();
	void send_identification();
	void process_command(uint8_t cmd);
};

#endif /* AERORFSENSOR_H_ */
