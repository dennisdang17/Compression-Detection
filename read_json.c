#include<stdio.h>
#include<json-c/json.h>

int main(int argc, char **argv) {
	FILE *fp;
	char buffer[1024]; //might need to change because our json file has more data

	struct json_object *parsed_json; //structure that holds parsed JSON
	//stores rest of fields of the JSON file
	struct json_object *Server_IP_Address;
	struct json_object *Source_Port_Number_UDP;
	struct json_object *Destination_Port_Number_TCP_Head;
	struct json_object *Destination_Port_Number_TCP_Tail;
	struct json_object *Port_Number_TCP;
	struct json_object *Size_UDP_Payload;
	struct json_object *Inter_Measurement_Time;
	struct json_object *Number_UDP_Packets;
	struct json_object *TTL_UDP_Packets;

	fp = fopen("myconfig.json","r"); //opens file
	fread(buffer, 1024, 1, fp); //reads files and puts contents inside buffer
	fclose(fp);

	parsed_json = json_tokener_parse(buffer); //parse json file's contents and convert them into a json object

	json_object_object_get_ex(parsed_json, "Server_IP_Address", &Server_IP_Address);
	json_object_object_get_ex(parsed_json, "Source_Port_Number_UDP", &Source_Port_Number_UDP);
	json_object_object_get_ex(parsed_json, "Destination_Port_Number_TCP_Head", &Destination_Port_Number_TCP_Head);
	json_object_object_get_ex(parsed_json, "Destination_Port_Number_TCP_Tail", &Destination_Port_Number_TCP_Tail);
	json_object_object_get_ex(parsed_json, "Port_Number_TCP", &Port_Number_TCP);
	json_object_object_get_ex(parsed_json, "Size_UDP_Payload", &Size_UDP_Payload);
	json_object_object_get_ex(parsed_json, "Inter_Measurement_Time", &Inter_Measurement_Time);
	json_object_object_get_ex(parsed_json, "Number_UDP_Packets", &Number_UDP_Packets);
	json_object_object_get_ex(parsed_json, "TTL_UDP_Packets", &TTL_UDP_Packets);

	printf("Server_IP_Address: %s\n", json_object_get_string(Server_IP_Address));
	printf("Source_Port_Number_UDP: %s\n", json_object_get_string(Source_Port_Number_UDP));
	printf("Destination_Port_Number_TCP_Head: %s\n", json_object_get_string(Destination_Port_Number_TCP_Head));
	printf("Destination_Port_Number_TCP_Tail: %s\n", json_object_get_string(Destination_Port_Number_TCP_Tail));
	printf("Port_Number_TCP: %s\n", json_object_get_string(Port_Number_TCP));
	printf("Size_UDP_Payload: %s\n", json_object_get_string(Size_UDP_Payload));
	printf("Inter_Measurement_Time: %s\n", json_object_get_string(Inter_Measurement_Time));
	printf("Number_UDP_Packets: %d\n", json_object_get_int(Number_UDP_Packets));
	printf("TTL_UDP_Packets: %d\n", json_object_get_int(TTL_UDP_Packets));
}
