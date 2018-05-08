#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <cctype>
#include <thread>
#include <chrono>
#include <signal.h>
#include <mqtt/client.h>
#include "camera.h"
#include "monitor.h"
#include "mjpegcamera.h"
#include "bundle.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;


const string SERVER_ADDRESS	{ "ssl://222.201.144.236:8883" };
const string CLIENT_ID		{ "1" };
const string TOPIC 			{ "/1/cmd" };
const int  QOS = 1;

Camera *pcamera;
MjpegCamera *pmcamera;
Monitor *pmonitor;
Bundle *pbundle;

typedef void (*sighandler_t)(int);
void sighandler(int sig){
	exit(1);
}

void cleanup(void){
	delete pcamera;
	delete pmcamera;
	delete pmonitor;
	delete pbundle;
}

int main(){
	//daemon(1,0);

	if(signal(SIGINT, sighandler)==SIG_ERR){
		perror("signal");
		return 1;
	}
	if(signal(SIGTERM, sighandler)==SIG_ERR){
		perror("signal");
		return 1;
	}

	if(atexit(cleanup)!=0){
		perror("atexit");
		return 1;
	}

    mqtt::connect_options connOpts("dev1", "dev1");
	connOpts.set_keep_alive_interval(20);
	connOpts.set_clean_session(true);
	mqtt::ssl_options sslopts;
	sslopts.set_trust_store("controller/res/ca.crt");
	connOpts.set_ssl(sslopts);

    mqtt::client cli(SERVER_ADDRESS, CLIENT_ID);

	pcamera = new Camera ("controller/src/publish/camera.sh");
	pmcamera = new MjpegCamera(cli, "127.0.0.1", 8080, "/1/mcamera/0", "controller/src/publish/mcamera.sh");
	pmonitor = new Monitor(cli, "127.0.0.1", 12307, "controller/src/publish/monitor.sh");
	pbundle = new Bundle(cli, "/1/sonar/0/realtime", "/1/laser/0/realtime", "/1/gesture/0/realtime");
	
	while(true){
		try {
			cli.connect(connOpts);
			cli.start_consuming();
			cli.subscribe(TOPIC, QOS);

			// Consume messages

			while (true) {
				puts("before consume_message");
				auto msg = cli.consume_message();
				if (!msg) break;
				cout << msg->get_topic() << ": " << msg->to_string() << endl;
				
				string cmd = msg->to_string();
				if(cmd == "camera_open"){
					pcamera->open();
				}else if(cmd == "camera_close"){
					pcamera->close();
				}else if(cmd == "monitor_open"){
					pmonitor->open();
				}else if(cmd == "monitor_close"){
					pmonitor->close();
				}else if(cmd == "mcamera_open"){
					pmcamera->open();
				}else if(cmd == "mcamera_close"){
					pmcamera->close();
				}else if(cmd == "bundle_open"){
					pbundle->open();
				}else if(cmd=="bundle_close"){
					pbundle->close();
				}
			}

			// Disconnect

			cout << "\nShutting down and disconnecting from the MQTT server..." << flush;
			cli.unsubscribe(TOPIC);
			cli.stop_consuming();
			cli.disconnect();
			cout << "OK" << endl;
		}
		catch (const mqtt::exception& exc) {
			cerr << exc.what() << endl;
		}
	}

    return 0;
}
