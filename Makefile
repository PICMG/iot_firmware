
all : libraries userver pldm_server pdrmaker

libraries:
	cd ./avr/lib/fcs && make clean
	cd ./avr/lib/pldm && make clean
	cd ./avr/lib/mctp && make clean
	cd ./avr/lib/uart && make clean
	cd ./avr/lib/fcs && make build
	cd ./avr/lib/pldm && make build
	cd ./avr/lib/mctp && make build
	cd ./avr/lib/uart && make build
	
userver : 
	cd ./avr/test/userver && make build

pdrmaker : 
	cd ./src/pdrmaker && make clean && make build

pldm_server : 
	cd ./avr/test/pldm_server && make build

avr_uart_test :
	cd ./avr/test/uart_test && make clean
	cd ./avr/test/uart_test && make build

clean:
	cd ./avr/lib/fcs && make clean
	cd ./avr/lib/uart && make clean
	cd ./avr/lib/mctp && make clean
	cd ./avr/lib/pldm && make clean
	cd ./avr/test/uart && make clean
	cd ./avr/test/mctp && make clean
	cd ./avr/avr/test/uart_test && make clean