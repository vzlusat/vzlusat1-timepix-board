#include <stdio.h>
#include <stdlib.h>
#include <command/command.h>
#include <csp/csp.h>
#include <csp/csp_endian.h>
#include <io/xray.h>
#include <io/nanomind.h>

int xray_cmd_power(struct command_context *ctx)
{
  if (ctx->argc < 2)
    return CMD_ERROR_SYNTAX;

  int on = atoi(ctx->argv[1]);
  if (obc_payloads_power_set(OBC_PAYLOADS_POWER_XRAY, on ? 1 : 0) != 0)
    return CMD_ERROR_FAIL;
  return CMD_ERROR_NONE;
}

// Get X-Ray board housekeeping data [Direct response]
int xray_cmd_get_hk_direct(struct command_context *ctx) {
	
	xray_msg_cmd_t msg;
	hk_data_t rep;
	
	msg.cmd = MEDIPIX_GET_HOUSEKEEPING;
  
	(void) ctx; //un-used
  
	int rv;
	rv = csp_transaction(CSP_PRIO_NORM, NODE_XRAY, XRAY_PORT_DIRECT, 2500, &msg, sizeof(xray_msg_cmd_t), &rep, sizeof(hk_data_t));
	
	if (rv <= 0) {
		
		printf("X-Ray get housekeeping error!\r\n");
		return CMD_ERROR_FAIL;
	
	} else {
	
		printf("X-Ray housekeeping:\r\n");
		printf("Boot counter: %d\r\n", csp_ntoh16(rep.bootCount));
		printf("Image counter: %d\r\n", csp_ntoh16(rep.imagesTaken));
		printf("Act. temperature: %d\r\n", rep.temperature);
		printf("Max. temperature: %d\r\n", rep.temperature_max);
		printf("Min. temperature: %d\r\n", rep.temperature_min);
		
		if (rep.framStatus == 1)
			printf("Fram OK\r\n");
		else
			printf("Fram ERROR!\r\n");
		
		if (rep.medipixStatus == 1) 
			printf("Medipix OK\r\n");
		else
			printf("Medipix ERROR!\r\n");
			
		printf("Runtime: %dh %dm %ds\r\n", csp_ntoh32(rep.seconds)/3600, csp_ntoh32(rep.seconds)/60, csp_ntoh32(rep.seconds)%60);
		printf("TIR_max: %i\r\n", (int16_t) csp_ntoh16(rep.TIR_max));
		printf("TIR_min: %i\r\n", (int16_t) csp_ntoh16(rep.TIR_min));
		printf("IR_max: %i\r\n", (int16_t) csp_ntoh16(rep.IR_max));
		printf("IR_min: %i\r\n", (int16_t) csp_ntoh16(rep.IR_min));
		printf("UV1_max: %i\r\n", (int16_t) csp_ntoh16(rep.UV1_max));
		printf("UV1_min: %i\r\n", (int16_t) csp_ntoh16(rep.UV1_min));
		printf("UV2_max: %i\r\n", (int16_t) csp_ntoh16(rep.UV2_max)); 
		printf("UV2_min: %i\r\n", (int16_t) csp_ntoh16(rep.UV2_min));
		
	}
	
	return CMD_ERROR_NONE;
}

// Get X-Ray board housekeeping data [DataKeeper response]
int xray_cmd_get_hk(struct command_context *ctx) {
	
	xray_msg_cmd_t msg;
	char buff[100];
	
	msg.cmd = MEDIPIX_GET_HOUSEKEEPING;
  
	(void) ctx; //un-used
  
	int rv;
	rv = csp_transaction(CSP_PRIO_NORM, NODE_XRAY, XRAY_PORT_DK, 2500, &msg, sizeof(xray_msg_cmd_t), &buff, -1);
	
	if (rv <= 0) {
		
		printf("X-Ray hk error!\r\n");
		return CMD_ERROR_FAIL;
	
	} else {
	
		buff[rv] = 0;
		printf("%s\r\n", buff);
	
	}
	
	return CMD_ERROR_NONE;
}

// Get temperature on Medipix's heatsink
int xray_cmd_get_temperature(struct command_context *ctx) {
			
	xray_msg_cmd_t msg;
	xray_temperature_t rep;
	
	msg.cmd = MEDIPIX_GET_TEMPERATURE;
  
	(void) ctx; //un-used
  
	int rv;
	rv = csp_transaction(CSP_PRIO_NORM, NODE_XRAY, XRAY_PORT_DIRECT, 2500, &msg, sizeof(xray_msg_cmd_t), &rep, sizeof(xray_temperature_t));
	
	if (rv <= 0) {
		
		printf("X-Ray get temperature error!\r\n");
		return CMD_ERROR_FAIL;
	
	} else {
	
		printf("Temperature is %d deg\r\n", rep.temp);
	
	}
	
	return CMD_ERROR_NONE;
}

// Save the Medipix's bootup message to DataKeeper
int xray_cmd_get_bootup_message(struct command_context *ctx) {
			
	xray_msg_cmd_t msg;
	char buff[100];
	
	msg.cmd = MEDIPIX_GET_BOOTUP_MESSAGE;
  
	(void) ctx; //un-used
  
	int rv;
	rv = csp_transaction(CSP_PRIO_NORM, NODE_XRAY, XRAY_PORT_DK, 5000, &msg, sizeof(xray_msg_cmd_t), &buff, -1);
	
	if (rv <= 0) {
		
		printf("X-Ray get bootup message error!\r\n");
		return CMD_ERROR_FAIL;
	
	} else {
	
		buff[rv] = 0;
		printf("%s\r\n", buff);
	
	}
	
	return CMD_ERROR_NONE;
}

// Initialize Medipix
int xray_cmd_medipix_init(struct command_context *ctx) {
			
	xray_msg_cmd_t msg;
	char buff[100];
	
	msg.cmd = MEDIPIX_PWR_ON;
  
	(void) ctx; //un-used
  
	int rv;
	rv = csp_transaction(CSP_PRIO_NORM, NODE_XRAY, XRAY_PORT_DIRECT, 12000, &msg, sizeof(xray_msg_cmd_t), &buff, -1);
	
	if (rv <= 0) {
		
		printf("X-Ray medipix init error!\r\n");
		return CMD_ERROR_FAIL;
	
	} else {
	
		buff[rv] = 0;
		printf("%s\r\n", buff);
	
	}
	
	return CMD_ERROR_NONE;
}

// ShutDown Medipix
int xray_cmd_medipix_shutdown(struct command_context *ctx) {
			
	xray_msg_cmd_t msg;
	char buff[100];
	
	msg.cmd = MEDIPIX_PWR_OFF;
  
	(void) ctx; //un-used
  
	int rv;
	rv = csp_transaction(CSP_PRIO_NORM, NODE_XRAY, XRAY_PORT_DIRECT, 2000, &msg, sizeof(xray_msg_cmd_t), &buff, -1);
	
	if (rv <= 0) {
		
		printf("X-Ray medipix shutdown error!\r\n");
		return CMD_ERROR_FAIL;
	
	} else {
	
		buff[rv] = 0;
		printf("%s\r\n", buff);
	
	}
	
	return CMD_ERROR_NONE;
}

// Get metadata of the last image [Direct Response]
int xray_cmd_get_metadata_direct(struct command_context *ctx) {
			
	xray_msg_cmd_t msg;
	imageParameters_t params;
	
	msg.cmd = MEDIPIX_SEND_METADATA;
  
	(void) ctx; //un-used
  
	int rv;
	rv = csp_transaction(CSP_PRIO_NORM, NODE_XRAY, XRAY_PORT_DIRECT, 2000, &msg, sizeof(xray_msg_cmd_t), &params, sizeof(imageParameters_t));
	
	if (rv <= 0) {
		
		printf("X-Ray metadata readout error!\r\n");
		return CMD_ERROR_FAIL;
	
	} else {
	
		printf("Metadata:\r\n");
		printf("Image ID: %u\r\n", csp_ntoh16(params.imageId));
		
		printf("Detector Mode: %s\r\n", (params.mode ? "TOT" : "MPX"));
		
		printf("Threshold: %u\r\n", csp_ntoh16(params.threshold));
		printf("Bias: %u\r\n", params.bias);
		printf("Exposure: %u\r\n", csp_ntoh16(params.exposure));
		printf("Filtering: %s\r\n", (params.filtering ? "ON" : "OFF"));
		printf("Output format: %u\r\n", params.outputForm);
		printf("Pixels (in filtered): %u\r\n", csp_ntoh16(params.nonZeroPixelsFiltered));
		printf("Pixels (in original): %u\r\n", csp_ntoh16(params.nonZeroPixelsOriginal));
		printf("Min value (original): %u\r\n", params.minValueOriginal);
		printf("Max value (original): %u\r\n", params.maxValueOriginal);
		printf("Min value (filtered): %u\r\n", params.minValueFiltered);
		printf("Max value (filtered): %u\r\n", params.maxValueFiltered);
		printf("Temperature: %u\r\n", params.temperature);
		printf("Time: %u\r\n", params.time);
		printf("Temperature limit: %d\r\n", params.temperatureLimit);
		printf("Pixel count thl: %u\r\n", csp_ntoh16(params.pixelCountThr));
		printf("UV thl: %i\r\n", (int16_t) csp_ntoh16(params.uv1_treshold));
		printf("Chunk ID: %u\r\n", csp_ntoh32(params.chunkId));
		printf("Attitude: [%d %d %d %d %d %d %d]\r\n", (int16_t) csp_ntoh16(params.attitude[0]), (int16_t) csp_ntoh16(params.attitude[1]),
		(int16_t) csp_ntoh16(params.attitude[2]), (int16_t) csp_ntoh16(params.attitude[3]), (int16_t) csp_ntoh16(params.attitude[4]), (int16_t) csp_ntoh16(params.attitude[5]), (int16_t) csp_ntoh16(params.attitude[6]));
		printf("Position: [%i %i %i]\r\n", (int16_t) csp_ntoh16(params.position[0]), (int16_t) csp_ntoh16(params.position[1]), (int16_t) csp_ntoh16(params.position[2]));
	}
	
	return CMD_ERROR_NONE;
}

// Get UV and IR sensors data
int xray_cmd_get_sensor_data_direct(struct command_context *ctx) {
			
	xray_msg_cmd_t msg;
	sensors_t sensors;
	
	msg.cmd = MEDIPIX_SEND_SENSOR_DATA;
  
	(void) ctx; //un-used
  
	int rv;
	rv = csp_transaction(CSP_PRIO_NORM, NODE_XRAY, XRAY_PORT_DIRECT, 2000, &msg, sizeof(xray_msg_cmd_t), &sensors, sizeof(sensors_t));
	
	if (rv <= 0) {
		
		printf("X-Ray sensors readout error!\r\n");
		return CMD_ERROR_FAIL;
	
	} else {
	
		printf("TIR: %i\r\n", (int16_t) csp_ntoh16(sensors.TIR));
		printf("IR: %i\r\n", (int16_t) csp_ntoh16(sensors.IR));
		printf("UV1: %i\r\n", (int16_t) csp_ntoh16(sensors.UV1));
		printf("UV2: %i\r\n", (int16_t) csp_ntoh16(sensors.UV2));
	}
	
	return CMD_ERROR_NONE;
}

// Set the Medipix's aquisition mode
int xray_cmd_medipix_setmode(struct command_context *ctx) {

	if (ctx->argc < 2)
		return CMD_ERROR_SYNTAX;		
			
	xray_msg_cmd_1byte_t msg;
	char buff[100];
	
	msg.cmd = MEDIPIX_SET_MODE;
	msg.data = atoi(ctx->argv[1]);
	
	if (msg.data > 1) {
			
		printf("Mode out of bounds [0, 1]\r\n");
		return CMD_ERROR_FAIL;
	}
  
	(void) ctx; //un-used
  
	int rv;
	rv = csp_transaction(CSP_PRIO_NORM, NODE_XRAY, XRAY_PORT_DIRECT, 12000, &msg, sizeof(xray_msg_cmd_1byte_t), &buff, -1);
	
	if (rv <= 0) {
		
		printf("X-Ray setting mode error!\r\n");
		return CMD_ERROR_FAIL;
	
	} else {
	
		buff[rv] = 0;
		printf("%s\r\n", buff);
	
	}
	
	return CMD_ERROR_NONE;
}

// Set the energy threshold
int xray_cmd_medipix_setthreshold(struct command_context *ctx) {

	if (ctx->argc < 2)
		return CMD_ERROR_SYNTAX;		
			
	xray_msg_cmd_1uin16_t msg;
	char buff[100];
	
	msg.cmd = MEDIPIX_SET_THRESHOLD;
	msg.data = atoi(ctx->argv[1]);
  
	if (msg.data > THRESHOLD_UPPER_BOUND) {
			
		printf("Thr out of bounds [0, 500]\r\n");
		return CMD_ERROR_FAIL;
	}
  
	(void) ctx; //un-used
  
	msg.data = csp_hton16(msg.data);
  
	int rv;
	rv = csp_transaction(CSP_PRIO_NORM, NODE_XRAY, XRAY_PORT_DIRECT, 2000, &msg, sizeof(xray_msg_cmd_1uin16_t), &buff, -1);
	
	if (rv <= 0) {
		
		printf("X-Ray setting thr error!\r\n");
		return CMD_ERROR_FAIL;
	
	} else {
	
		buff[rv] = 0;
		printf("%s\r\n", buff);
	
	}
	
	return CMD_ERROR_NONE;
}

// set the bias
int xray_cmd_medipix_setbias(struct command_context *ctx) {

	if (ctx->argc < 2)
		return CMD_ERROR_SYNTAX;		
			
	xray_msg_cmd_1byte_t msg;
	char buff[100];
	
	msg.cmd = MEDIPIX_SET_BIAS;
	msg.data = atoi(ctx->argv[1]);
  
	(void) ctx; //un-used
  
	int rv;
	rv = csp_transaction(CSP_PRIO_NORM, NODE_XRAY, XRAY_PORT_DIRECT, 2000, &msg, sizeof(xray_msg_cmd_1byte_t), &buff, -1);
	
	if (rv <= 0) {
		
		printf("X-Ray setting bias error!\r\n");
		return CMD_ERROR_FAIL;
	
	} else {
	
		buff[rv] = 0;
		printf("%s\r\n", buff);
	
	}
	
	return CMD_ERROR_NONE;
}

// set the exposure time
int xray_cmd_medipix_setexposure(struct command_context *ctx) {

	if (ctx->argc < 2)
		return CMD_ERROR_SYNTAX;		
			
	xray_msg_cmd_1uin16_t msg;
	char buff[100];
	
	msg.cmd = MEDIPIX_SET_EXPOSURE;
	msg.data = csp_hton16(atoi(ctx->argv[1]));
  
	(void) ctx; //un-used
  
	int rv;
	rv = csp_transaction(CSP_PRIO_NORM, NODE_XRAY, XRAY_PORT_DIRECT, 2000, &msg, sizeof(xray_msg_cmd_1uin16_t), &buff, -1);
	
	if (rv <= 0) {
		
		printf("X-Ray setting exposure error!\r\n");
		return CMD_ERROR_FAIL;
	
	} else {
	
		buff[rv] = 0;
		printf("%s\r\n", buff);
	
	}
	
	return CMD_ERROR_NONE;
}

// set the temperature limit
int xray_cmd_medipix_settemplimit(struct command_context *ctx) {

	if (ctx->argc < 2)
		return CMD_ERROR_SYNTAX;		
			
	xray_msg_cmd_int8_t msg;
	char buff[100];
	
	msg.cmd = MEDIPIX_SET_TEMPLIMIT;
	msg.data = atoi(ctx->argv[1]);
  
	(void) ctx; //un-used
  
	int rv;
	rv = csp_transaction(CSP_PRIO_NORM, NODE_XRAY, XRAY_PORT_DIRECT, 2000, &msg, sizeof(xray_msg_cmd_int8_t), &buff, -1);
	
	if (rv <= 0) {
		
		printf("X-Ray setting temp limit error!\r\n");
		return CMD_ERROR_FAIL;
	
	} else {
	
		buff[rv] = 0;
		printf("%s\r\n", buff);
	
	}
	
	return CMD_ERROR_NONE;
}

// set the pixel count threshold
int xray_cmd_medipix_set_pxlcntthr(struct command_context *ctx) {

	if (ctx->argc < 2)
		return CMD_ERROR_SYNTAX;		
			
	xray_msg_cmd_1uin16_t msg;
	char buff[100];
	
	msg.cmd = MEDIPIX_SET_PIXELCNTTHR;
	msg.data = csp_hton16(atoi(ctx->argv[1]));
  
	(void) ctx; //un-used
  
	int rv;
	rv = csp_transaction(CSP_PRIO_NORM, NODE_XRAY, XRAY_PORT_DIRECT, 2000, &msg, sizeof(xray_msg_cmd_1uin16_t), &buff, -1);
	
	if (rv <= 0) {
		
		printf("X-Ray setting pxl cnt thr error!\r\n");
		return CMD_ERROR_FAIL;
	
	} else {
	
		buff[rv] = 0;
		printf("%s\r\n", buff);
	
	}
	
	return CMD_ERROR_NONE;
}

// set the UV1 threshold
int xray_cmd_medipix_set_uvthreshold(struct command_context *ctx) {

	if (ctx->argc < 2)
		return CMD_ERROR_SYNTAX;		
			
	xray_msg_cmd_1in16_t msg;
	char buff[100];
	
	msg.cmd = MEDIPIX_SET_UV1THL;
	msg.data = csp_hton16(atoi(ctx->argv[1]));
  
	(void) ctx; //un-used
  
	int rv;
	rv = csp_transaction(CSP_PRIO_NORM, NODE_XRAY, XRAY_PORT_DIRECT, 2000, &msg, sizeof(xray_msg_cmd_1uin16_t), &buff, -1);
	
	if (rv <= 0) {
		
		printf("X-Ray setting UV thr error!\r\n");
		return CMD_ERROR_FAIL;
	
	} else {
	
		buff[rv] = 0;
		printf("%s\r\n", buff);
	
	}
	
	return CMD_ERROR_NONE;
}

// set filtering (1 = on, 0 = off)
int xray_cmd_medipix_setfiltering(struct command_context *ctx) {

	if (ctx->argc < 2)
		return CMD_ERROR_SYNTAX;		
			
	xray_msg_cmd_1byte_t msg;
	char buff[100];
	
	msg.cmd = MEDIPIX_SET_FILTERING;
	msg.data = atoi(ctx->argv[1]);
  
	if (msg.data > 1) {
			
		printf("Fil out of bounds [0, 1]\r\n");
		return CMD_ERROR_FAIL;
	}
  
	(void) ctx; //un-used
  
	int rv;
	rv = csp_transaction(CSP_PRIO_NORM, NODE_XRAY, XRAY_PORT_DIRECT, 2000, &msg, sizeof(xray_msg_cmd_1byte_t), &buff, -1);
	
	if (rv <= 0) {
		
		printf("X-Ray setting filtering error!\r\n");
		return CMD_ERROR_FAIL;
	
	} else {
	
		buff[rv] = 0;
		printf("%s\r\n", buff);
	
	}
	
	return CMD_ERROR_NONE;
}

// set the output method
int xray_cmd_medipix_setoutputform(struct command_context *ctx) {

	if (ctx->argc < 2)
		return CMD_ERROR_SYNTAX;		
			
	xray_msg_cmd_1byte_t msg;
	char buff[100];
	
	msg.cmd = MEDIPIX_SET_OUTPUT_FORM;
	msg.data = atoi(ctx->argv[1]);
	
	if (msg.data > 63) {
			
		printf("Outform out of bounds [0, 63]\r\n");
		return CMD_ERROR_FAIL;
	}
  
	(void) ctx; //un-used
  
	int rv;
	rv = csp_transaction(CSP_PRIO_NORM, NODE_XRAY, XRAY_PORT_DIRECT, 2000, &msg, sizeof(xray_msg_cmd_1byte_t), &buff, -1);
	
	if (rv <= 0) {
		
		printf("X-Ray setting outputform error!\r\n");
		return CMD_ERROR_FAIL;
	
	} else {
	
		buff[rv] = 0;
		printf("%s\r\n", buff);
	
	}
	
	return CMD_ERROR_NONE;
}

// set all parameters
int xray_cmd_medipix_setallparameters(struct command_context *ctx) {

	if (ctx->argc < 9)
		return CMD_ERROR_SYNTAX;		
			
	xray_msg_cmd_params_t msg;
	char buff[100];
	
	msg.cmd = MEDIPIX_SET_ALL_PARAMS;
	msg.data.treshold = atoi(ctx->argv[1]);
	msg.data.exposure = atoi(ctx->argv[2]);
	msg.data.bias = atoi(ctx->argv[3]);
	msg.data.filtering = atoi(ctx->argv[4]);
	msg.data.mode = atoi(ctx->argv[5]);
	msg.data.outputForm = atoi(ctx->argv[6]);
	msg.data.temperatureLimit = atoi(ctx->argv[7]);
	msg.data.pixelCountThr = atoi(ctx->argv[8]);
	msg.data.uv1_treshold = atoi(ctx->argv[9]);
	  
	if (msg.data.treshold > THRESHOLD_UPPER_BOUND) {
			
		printf("Thr out of bounds [0, 500]\r\n");
		return CMD_ERROR_FAIL;
	}
	
	if (msg.data.filtering > 1) {
			
		printf("Fil out of bounds [0, 1]\r\n");
		return CMD_ERROR_FAIL;
	}
	
	if (msg.data.mode > 1) {
			
		printf("Mode out of bounds [0, 1]\r\n");
		return CMD_ERROR_FAIL;
	}
	
	if (msg.data.outputForm > 63) {
			
		printf("Outform out of bounds [0, 63]\r\n");
		return CMD_ERROR_FAIL;
	}
	  
	(void) ctx; //un-used
	
	msg.data.treshold = csp_hton16(msg.data.treshold);
	msg.data.exposure = csp_hton16(msg.data.exposure);
	msg.data.pixelCountThr = csp_hton16(msg.data.pixelCountThr);
	msg.data.uv1_treshold = csp_hton16(msg.data.uv1_treshold);
  
	int rv;
	rv = csp_transaction(CSP_PRIO_NORM, NODE_XRAY, XRAY_PORT_DIRECT, 12000, &msg, sizeof(xray_msg_cmd_params_t), &buff, -1);
	
	if (rv <= 0) {
		
		printf("X-Ray setting parameters error!\r\n");
		return CMD_ERROR_FAIL;
	
	} else {
	
		buff[rv] = 0;
		printf("%s\r\n", buff);
	
	}
	
	return CMD_ERROR_NONE;
}

// measurement, direct output, turnoff after aquisition
int xray_cmd_medipix_measuredirect(struct command_context *ctx) {
			
	xray_msg_cmd_t msg;
	char buff[100];
	
	msg.cmd = MEDIPIX_MEASURE;
  
	(void) ctx; //un-used
  
	csp_transaction(CSP_PRIO_NORM, NODE_XRAY, XRAY_PORT_DIRECT, 2000, &msg, sizeof(xray_msg_cmd_t), &buff, -1);
	
	return CMD_ERROR_NONE;
}

// measurement, DK output, turnoff after aquisition
int xray_cmd_medipix_measure(struct command_context *ctx) {
			
	xray_msg_cmd_t msg;
	char buff[100];
	
	msg.cmd = MEDIPIX_MEASURE;
  
	(void) ctx; //un-used
  
	int rv;
	rv = csp_transaction(CSP_PRIO_NORM, NODE_XRAY, XRAY_PORT_DK, 2000, &msg, sizeof(xray_msg_cmd_t), &buff, -1);
	
	if (rv <= 0) {
		
		printf("X-Ray measuring error!\r\n");
		return CMD_ERROR_FAIL;
	
	} else {
	
		buff[rv] = 0;
		printf("%s\r\n", buff);
	
	}
	
	return CMD_ERROR_NONE;
}

// measurement, DK output, turnoff after aquisition, UV triggered
int xray_cmd_measure_uv_trigger(struct command_context *ctx) {
			
	xray_msg_cmd_t msg;
	char buff[100];
	
	msg.cmd = MEDIPIX_MEASURE_UV;
  
	(void) ctx; //un-used
  
	int rv;
	rv = csp_transaction(CSP_PRIO_NORM, NODE_XRAY, XRAY_PORT_DK, 2000, &msg, sizeof(xray_msg_cmd_t), &buff, -1);
	
	if (rv <= 0) {
		
		printf("X-Ray measuring error!\r\n");
		return CMD_ERROR_FAIL;
	
	} else {
	
		buff[rv] = 0;
		printf("%s\r\n", buff);
	
	}
	
	return CMD_ERROR_NONE;
}

// measurement, DK output, turnoff after aquisition, uses the pixel limit
int xray_cmd_medipix_measure_scanning_mode(struct command_context *ctx) {
			
	xray_msg_cmd_t msg;
	char buff[100];
	
	msg.cmd = MEDIPIX_MEASURE_SCANNING_MODE;
  
	(void) ctx; //un-used
  
	int rv;
	rv = csp_transaction(CSP_PRIO_NORM, NODE_XRAY, XRAY_PORT_DK, 2000, &msg, sizeof(xray_msg_cmd_t), &buff, -1);
	
	if (rv <= 0) {
		
		printf("X-Ray measuring error!\r\n");
		return CMD_ERROR_FAIL;
	
	} else {
	
		buff[rv] = 0;
		printf("%s\r\n", buff);
	
	}
	
	return CMD_ERROR_NONE;
}

// measurement, DK output, no-turnoff after aquisition, uses the pixel limit
int xray_cmd_medipix_measure_scanning_mode_no_turnoff(struct command_context *ctx) {
			
	xray_msg_cmd_t msg;
	char buff[100];
	
	msg.cmd = MEDIPIX_MEASURE_SCANNING_MODE_NO_TURNOFF;
  
	(void) ctx; //un-used
  
	int rv;
	rv = csp_transaction(CSP_PRIO_NORM, NODE_XRAY, XRAY_PORT_DK, 2000, &msg, sizeof(xray_msg_cmd_t), &buff, -1);
	
	if (rv <= 0) {
		
		printf("X-Ray measuring error!\r\n");
		return CMD_ERROR_FAIL;
	
	} else {
	
		buff[rv] = 0;
		printf("%s\r\n", buff);
	
	}
	
	return CMD_ERROR_NONE;
}

// measurement, direct output, NO-turnoff after aquisition
int xray_cmd_medipix_measuredirect_no_turnoff(struct command_context *ctx) {
			
	xray_msg_cmd_t msg;
	char buff[100];
	
	msg.cmd = MEDIPIX_MEASURE_NO_TURNOFF;
  
	(void) ctx; //un-used
  
	csp_transaction(CSP_PRIO_NORM, NODE_XRAY, XRAY_PORT_DIRECT, 2000, &msg, sizeof(xray_msg_cmd_t), &buff, -1);
	
	return CMD_ERROR_NONE;
}

// measurement, DK output, NO-turnoff after aquisition
int xray_cmd_medipix_measure_no_turnoff(struct command_context *ctx) {
			
	xray_msg_cmd_t msg;
	char buff[100];
	
	msg.cmd = MEDIPIX_MEASURE_NO_TURNOFF;
  
	(void) ctx; //un-used
  
	int rv;
	rv = csp_transaction(CSP_PRIO_NORM, NODE_XRAY, XRAY_PORT_DK, 2000, &msg, sizeof(xray_msg_cmd_t), &buff, -1);
	
	if (rv <= 0) {
		
		printf("X-Ray measuring error!\r\n");
		return CMD_ERROR_FAIL;
	
	} else {
	
		buff[rv] = 0;
		printf("%s\r\n", buff);
	
	}
	
	return CMD_ERROR_NONE;
}

// request to create all storages in DK
int xray_cmd_create_storages(struct command_context *ctx) {
			
	xray_msg_cmd_t msg;
	char buff[100];
	
	msg.cmd = XRAY_DK_CREATE_STORAGES;
  
	(void) ctx; //un-used
  
	int rv;
	rv = csp_transaction(CSP_PRIO_NORM, NODE_XRAY, XRAY_PORT_DIRECT, 3000, &msg, sizeof(xray_msg_cmd_t), &buff, -1);
	
	if (rv <= 0) {
		
		printf("X-Ray creating storages error!\r\n");
		return CMD_ERROR_FAIL;
	
	} else {
	
		buff[rv] = 0;
		printf("%s\r\n", buff);
	}
	
	return CMD_ERROR_NONE;
}

// request to get the original (non-filtered) image [direct response]
int xray_cmd_get_original_direct(struct command_context *ctx) {
			
	xray_msg_cmd_t msg;
	char buff[100];
	
	msg.cmd = MEDIPIX_SEND_ORIGINAL;
  
	(void) ctx; //un-used
  
	csp_transaction(CSP_PRIO_NORM, NODE_XRAY, XRAY_PORT_DIRECT, 1000, &msg, sizeof(xray_msg_cmd_t), &buff, -1);
	
	return CMD_ERROR_NONE;
}

command_t xray_commands[] = {
  {
    .name = "pwr",
    .help = "Turn on/off power for X-Ray board",
    .handler = xray_cmd_power,
  },
  {
    .name = "in",
    .help = "Power on and initialize Medipix",
    .handler = xray_cmd_medipix_init,
  },
  {
    .name = "sd",
    .help = "Shut down Medipix",
    .handler = xray_cmd_medipix_shutdown,
  },
   {
    .name = "bm",
    .help = "Get medipix bootup message [DK response]",
    .handler = xray_cmd_get_bootup_message,
  },
  {
    .name = "hkd",
    .help = "Get housekeeping data [DK response]",
    .handler = xray_cmd_get_hk,
  },
   {
    .name = "sm",
    .help = "Set aquisition mode (0 = MPX, 1 = TOT)",
    .handler = xray_cmd_medipix_setmode,
  },
   {
    .name = "st",
    .help = "Set threshold",
    .handler = xray_cmd_medipix_setthreshold,
  },
   {
    .name = "sb",
    .help = "Set bias [0 - 255]",
    .handler = xray_cmd_medipix_setbias,
  },
   {
    .name = "se",
    .help = "Set exposure [ms]",
    .handler = xray_cmd_medipix_setexposure,
  },
   {
    .name = "sf",
    .help = "Set filtering [0 = yes, 1 = no]",
    .handler = xray_cmd_medipix_setfiltering,
  },
   {
    .name = "so",
    .help = "Set output form [0 to 63]",
    .handler = xray_cmd_medipix_setoutputform,
  },
  {
    .name = "spc",
    .help = "Set pixel count thr",
    .handler = xray_cmd_medipix_set_pxlcntthr,
  },
  {
    .name = "stl",
    .help = "Set temperature limit [deg celsius]",
    .handler = xray_cmd_medipix_settemplimit,
  },
  {
    .name = "suv",
    .help = "Set UV thr",
    .handler = xray_cmd_medipix_set_uvthreshold,
  },
  {
    .name = "sp",
    .help = "Set all parameters [thr, exp, bia, fil, mod, out, tmp, pxl cnt, uv thr]",
    .handler = xray_cmd_medipix_setallparameters,
  },
  {
    .name = "m",
    .help = "Measure [DK response]",
    .handler = xray_cmd_medipix_measure,
  },
  {
    .name = "mu",
    .help = "Measure [UV Trigger]",
    .handler = xray_cmd_measure_uv_trigger,
  },
  {
    .name = "mn",
    .help = "Measure, no turnoff [DK response]",
    .handler = xray_cmd_medipix_measure_no_turnoff,
  },
  {
    .name = "ms",
    .help = "Measure, scannign mode [DK response]",
    .handler = xray_cmd_medipix_measure_scanning_mode,
  },
  {
    .name = "msn",
    .help = "Measure, scannign mode, no turnoff [DK response]",
    .handler = xray_cmd_medipix_measure_scanning_mode_no_turnoff,
  },
  {
    .name = "hk",
    .help = "Get housekeeping data [direct response]",
    .handler = xray_cmd_get_hk_direct,
  },
  {
    .name = "gt",
    .help = "Get temperature [direct response]",
    .handler = xray_cmd_get_temperature,
  },
  {
    .name = "gs",
    .help = "Get UV and IR sensors data [Direct response]",
    .handler = xray_cmd_get_sensor_data_direct,
  },
  {
    .name = "dm",
    .help = "Measure [direct response]",
    .handler = xray_cmd_medipix_measuredirect,
  },
  {
    .name = "dmn",
    .help = "Measure, no turnoff [direct response]",
    .handler = xray_cmd_medipix_measuredirect_no_turnoff,
  },
  {
    .name = "go",
    .help = "Get original image [Direct response]",
    .handler = xray_cmd_get_original_direct,
  },
  {
    .name = "gm",
    .help = "Get last image metadata [direct response]",
    .handler = xray_cmd_get_metadata_direct,
  },
  {
    .name = "cs",
    .help = "Create DK storages",
    .handler = xray_cmd_create_storages,
  },
};
command_t __root_command xray_comd[] = {
  {
    .name = "x",
    .help = "X-Ray board commands",
    .chain = INIT_CHAIN(xray_commands),
  }
};

void cmd_xray_setup()
{
  command_register(xray_comd);
}
