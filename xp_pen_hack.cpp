#include <fstream>
#include <iostream>
#include <linux/input.h>
#include <regex>
#include <time.h>
#include <optional>
#include <unistd.h>
#include <vector>
#include <memory>

bool verbosity = false;
int magic_evt_code = 331;

typedef struct device {
    std::string name;
    std::string evt_n;
} device_t;

enum PenStatus {
    LOW,
    HIGH
};

/*
std::optional<std::string> find_event_number(const char* device_name) {
    for (std::string line; getline(devices, line);) {
        if (line.find("XP-PEN DECO 02") == std::string::npos) {
            continue;
        }

        for (; line.rfind("H", 0) != 0; getline(devices, line)) {
        };


        for (; line.rfind("B: EV", 0) != 0 && line.rfind("I", 0) != 0;
                getline(devices, line)) {
        };

        if (line.rfind("B: EV=1b", 0) == 0)
            return number;
    }

    return {};
};

I: Bus=0003 Vendor=046d Product=c31c Version=0110
N: Name="Logitech USB Keyboard Consumer Control"
P: Phys=usb-0000:03:00.0-10/input1
S: Sysfs=/devices/pci0000:00/0000:00:01.3/0000:03:00.0/usb1/1-10/1-10:1.1/0003:046D:C31C.0004/input/input23
U: Uniq=
H: Handlers=kbd event21 
B: PROP=0
B: EV=1b
B: KEY=2010000 397ad8011001 e000000000000 0
B: ABS=100000000
B: MSC=10
*/

static const char* device_file_path = "/proc/bus/input/devices";

auto pack_devices(std::vector<device_t*>& devices) {

    std::ifstream device_file { std::string(device_file_path), std::ios::in };
    if (!device_file) {
        std::cerr << "Could not open device file!" << std::endl;
        exit(1);
    }

    std::string number;
    std::smatch matches;
    std::string name_key = "Name=\"";
    std::regex event_nr_rx("event([0-9]+)");

    auto get_names = [&]() -> bool {
        device_t* to_add = new device_t();
        for (std::string line; getline(device_file, line);) {
            auto found = line.find(name_key);
            if(found == std::string::npos) continue;
            else {
                found += name_key.size();
                to_add->name = line.substr(found, line.length() - 4 - name_key.size()); //used some magic to fit the string properly :^)
                for (std::string line; getline(device_file, line);) {
                    if (std::regex_search(line, matches, event_nr_rx)) {
                        to_add->evt_n = matches[1].str();
                        devices.push_back(to_add);
                        return true;
                    }
                }
            }
        }
        return false;
    };
    
    while(get_names());

    if(verbosity) {
        for(auto d : devices) {
            std::cout
                << "Device Name: " << d->name
                << "\nEvent Number: " << d->evt_n
                << "\n";
        }
    }

    return devices;
};

auto select(const std::vector<device_t*>& devices) -> device_t* {
    int no = -1;

    for(int i = 0; i < devices.size(); i++)
        std::cout << i << ") " << devices[i]->name << std::endl;

    do {
        std::cout << "\nSelect your tablet according to its device name: ";
        std::cin >> no;
    } while(no < 0 && no >= devices.size());

    return devices[no];
};

auto listen(device_t* selected_device) -> void {
    const auto event_device = std::string("/dev/input/event") + selected_device->evt_n;
    std::cout << "Using device: " << event_device << std::endl;
    std::ifstream events { event_device, std::ios::in | std::ios::binary };
    if (!events || getuid()) {
        std::cerr << "Could not open device!\nFile not valid or missing sudo rights" << std::endl;
        exit(1);
    }

    input_event event;

    while (events.read((char *)&event, sizeof(event))) {
        if (event.code == magic_evt_code) {
            if(event.value) {
                if(verbosity)
                    std::cout << "Xournal eraser, evt value: " << event.value << std::endl;
                system("xdotool key shift+ctrl+e");
            } else {
                if(verbosity)
                    std::cout << "Xournal pen, evt value: " << event.value << std::endl;
                system("xdotool key shift+ctrl+p");
            }
        }
    }
}

int main(int argc, char** argv) {
    if(argc > 1 && (strcmp(argv[1], "--verbose") || strcmp(argv[1], "-v")))
        verbosity = true;

    std::vector<device_t*> devices;
    device_t* selected_device;
    pack_devices(devices);
    listen(select(devices));
    return 0;
}
