VENV_DIR=/opt/ecofloc/gui/venv/
FLOC_DIR=/opt/ecofloc/
GUI_DIR=/opt/ecofloc/gui/
BIN_DIR=/usr/local/bin
REQUIREMENTS=python_libs.conf

all: clean uninstall cpu gpu sd ram nic floc install

create_ecofloc_folder: 
	mkdir -p $(FLOC_DIR)
create_gui_folder: 
	mkdir -p $(GUI_DIR)

######## CPU ##########

cpu:
	$(MAKE) -C ecofloc-cpu
install_cpu:
	cp ecofloc-cpu/ecofloc-cpu.out ecofloc-cpu/cpu_settings.conf $(FLOC_DIR)
clean_cpu:
	$(MAKE) -C ecofloc-cpu clean
uninstall_cpu:
	rm -f $(BIN_DIR)/ecofloc-cpu

######## GPU ##########

gpu:
	$(MAKE) -C ecofloc-gpu
install_gpu:
	cp ecofloc-gpu/ecofloc-gpu.out ecofloc-gpu/gpu_settings.conf $(FLOC_DIR)
clean_gpu:
	$(MAKE) -C ecofloc-gpu clean
uninstall_gpu:
	rm -f $(BIN_DIR)/ecofloc-gpu

######## SD ##########

sd:
	$(MAKE) -C ecofloc-sd
install_sd:
	cp ecofloc-sd/ecofloc-sd.out ecofloc-sd/sd_features.conf ecofloc-sd/sd_settings.conf $(FLOC_DIR)
clean_sd:
	$(MAKE) -C ecofloc-sd clean
uninstall_sd:
	rm -f $(BIN_DIR)/ecofloc-sd $(BIN_DIR)/sd_features.conf
	
######## RAM ##########

ram:
	$(MAKE) -C ecofloc-ram
install_ram:
	cp ecofloc-ram/ecofloc-ram.out ecofloc-ram/ram_settings.conf $(FLOC_DIR)
clean_ram:
	$(MAKE) -C ecofloc-ram clean
uninstall_ram:
	rm -f $(BIN_DIR)/ecofloc-ram

######## NIC ##########

nic:
	$(MAKE) -C ecofloc-nic
install_nic:
	cp ecofloc-nic/ecofloc-nic.out ecofloc-nic/nic_features.conf $(FLOC_DIR)
clean_nic:
	$(MAKE) -C ecofloc-nic clean
uninstall_nic:
	rm -f $(BIN_DIR)/ecofloc-nic

######## FLOC ##########

floc:
	gcc ecofloc.c -o ecofloc
install_floc:
	cp ecofloc $(FLOC_DIR)
	ln -sf $(FLOC_DIR)ecofloc $(BIN_DIR)/ecofloc
clean_floc:
	rm -f ecofloc
uninstall_floc:
	rm -rf $(BIN_DIR)/ecofloc


######## GUI ##########

# Main target to install the GUI
install_gui: create_gui_folder setup_venv copy_files
	chmod +x $(GUI_DIR)execute_gui.sh
	ln -sf $(GUI_DIR)execute_gui.sh $(BIN_DIR)/ecoflocUI

# Create and activate the virtual environment, then install the required packages
setup_venv: $(REQUIREMENTS)
	python3 -m venv $(VENV_DIR)
	. $(VENV_DIR)/bin/activate && pip install -r $(REQUIREMENTS)

# Copy GUI files
copy_files:
	cp gui/flocUI.py gui/energy_plotter.py gui/floc_daemon.py gui/power_plotter.py gui/system_monitor.py $(GUI_DIR)
	cp -r gui/assets $(GUI_DIR)
	cp gui/execute_gui.sh $(GUI_DIR)
	cp activities.yaml $(FLOC_DIR)

######## ALL ##########

clean:
	$(MAKE) -C ecofloc-cpu clean
	$(MAKE) -C ecofloc-gpu clean
	$(MAKE) -C ecofloc-ram clean
	$(MAKE) -C ecofloc-sd clean
	$(MAKE) -C ecofloc-nic clean
	rm -f ecofloc

install: create_ecofloc_folder install_cpu install_gpu install_sd install_ram install_nic install_floc
	chmod +x $(FLOC_DIR)ecofloc-cpu.out $(FLOC_DIR)ecofloc-gpu.out $(FLOC_DIR)ecofloc-sd.out $(FLOC_DIR)ecofloc-ram.out $(FLOC_DIR)ecofloc-nic.out $(FLOC_DIR)ecofloc
	chmod +x $(BIN_DIR)/ecofloc
uninstall:
	rm -rf $(FLOC_DIR)
	rm -f $(BIN_DIR)/floc $(BIN_DIR)/ecoflocUI $(BIN_DIR)/sd_features.conf