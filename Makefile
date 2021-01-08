all : libraries userver pldm_test uart_c_test mctp_c_test pldm_client pldm_server pldm_cmd_test

libraries:
	cd ./lib/json && make clean
	cd ./lib/fcs && make clean
	cd ./lib/pldm && make clean
	cd ./lib/mctp && make clean
	cd ./lib/uart && make clean
	cd ./lib/json && make build
	cd ./lib/fcs && make build
	cd ./lib/pldm && make build
	cd ./lib/mctp && make build
	cd ./lib/uart && make build
	
userver : 
	cd ./avr/test/userver && make build

pldm_client : 
	cd ./test/pldm_client && make build

pldm_cmd_test : 
	cd ./test/pldm_cmd_test && make build

pldm_server : 
	cd ./test/pldm_server && make build

pldm_test : 
	cd ./test/pldm && make clean
	cd ./test/pldm && make build

uart_c_test : 
	cd ./test/uart && make clean
	cd ./test/uart && make build

mctp_c_test : 
	cd ./test/mctp && make clean
	cd ./test/mctp && make build

avr_uart_test :
	cd ./avr/test/uart_test && make clean
	cd ./avr/test/uart_test && make build

clean:
	cd ./lib/fcs && make clean
	cd ./lib/uart && make clean
	cd ./lib/mctp && make clean
	cd ./lib/json && make clean
	cd ./lib/pldm && make clean
	cd ./test/pldm && make clean
	cd ./test/uart && make clean
	cd ./test/mctp && make clean
	cd ./avr/test/uart_test && make clean