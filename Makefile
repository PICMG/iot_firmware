pldm_test : 
	cd ./lib/json && make clean
	cd ./lib/pldm && make clean
	cd ./test/pldm && make clean
	cd ./test/pldm && make build

clean:
	cd ./lib/json && make clean
	cd ./lib/pldm && make clean
	cd ./test/pldm && make clean
