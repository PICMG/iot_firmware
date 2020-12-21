pldm_test : 
	cd ./lib/json && make clean
	cd ./lib/pldm && make clean
	cd ./test/pldm && make clean
	cd ./test/pldm && make build

uart_c_test : 
	cd ./lib/uartc && make clean
	cd ./test/uart_c_test && make clean
	cd ./test/uart_c_test && make build

mctp_c_test : 
	cd ./lib/mctpc && make clean
	cd ./lib/fcsc && make clean
	cd ./lib/uartc && make clean
	cd ./test/mctp_c_test && make clean
	cd ./test/mctp_c_test && make build

clean:
	cd ./lib/fcsc && make clean
	cd ./lib/uartc && make clean
	cd ./lib/mctpc && make clean
	cd ./lib/json && make clean
	cd ./lib/pldm && make clean
	cd ./test/pldm && make clean
	cd ./test/uart_c_test && make clean
	cd ./test/mctp_c_test && make clean
