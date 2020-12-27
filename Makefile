all : pldm_test uart_c_test mctp_c_test avr_uart_test

pldm_test : 
	cd ./lib/json && make clean
	cd ./lib/pldm && make clean
	cd ./test/pldm && make clean
	cd ./test/pldm && make build

uart_c_test : 
	cd ./lib/uart && make clean
	cd ./test/uart && make clean
	cd ./test/uart && make build

mctp_c_test : 
	cd ./lib/mctp && make clean
	cd ./lib/fcs && make clean
	cd ./lib/uart && make clean
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