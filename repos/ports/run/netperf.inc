#
# \brief  Test for using netperf
# \author Alexander Boettcher
# \date   2013-04-18
#

if {[have_include "power_on/qemu"]} {
	puts "\nNetperf running on Qemu is not recommended.\n"
	exit
}

if {[have_spec hw_odroid_xu]} {
	puts "Run script does not support this platform."
	exit 0
}

set use_usb_driver      [expr [have_spec omap4] || [have_spec platform_arndale] || [have_spec platform_rpi]]
set use_nic_driver      [expr !$use_usb_driver && ![have_spec platform_imx53] && !$use_wifi_driver]
set use_platform_driver [expr [have_spec platform_arndale] || [have_spec platform_imx53] || [have_spec platform_rpi]]

if {[expr !$use_usb_driver && !$use_nic_driver && !$use_wifi_driver]} {
	puts "\n Run script is not supported on this platform. \n"; exit 0 }

# provide wifi related variables in case we do not use the wifi driver
if {!$use_wifi_driver} {
	set wifi_ssid ""
	set wifi_bssid ""
	set wifi_psk ""
}

#
# Build
#

set version "2.6.0"

# sanity check that the right version is used
set wrong_version [catch {
	spawn netperf-$version -V
	expect {
		{Netperf version 2.6.0}  { }
		eof { return }
		timeout { return }
	}
}]

if {$wrong_version} {
	puts -nonewline "\nPlease compile a netperf client of version $version "
	puts "for your host system."
	puts "The sources are available in 'ports/contrib/netperf'."
	puts "Please name the binary netperf-$version\n"
	exit 1;
}

# netperf configuration
set packet_size 1024
set netperf_tests "TCP_STREAM TCP_MAERTS"

# start run script generation

if {$use_usb_driver}    { set network_driver "usb_drv" }
if {$use_nic_driver}    { set network_driver "nic_drv" }
if {$use_wifi_driver}   { set network_driver "wifi_drv" }

set build_components {
	core init
	drivers/pci drivers/timer
}

append     build_components     " $netperf_target "
lappend_if $use_nic_driver      build_components drivers/nic
lappend_if $use_usb_driver      build_components drivers/usb
lappend_if $use_platform_driver build_components drivers/platform
lappend_if $use_nic_bridge      build_components server/nic_bridge
lappend_if [have_spec acpi]     build_components drivers/acpi
lappend_if [have_spec pci]      build_components drivers/pci/device_pd
lappend_if [have_spec gpio]     build_components drivers/gpio

append_if $use_wifi_driver      build_components {
	drivers/wifi
	server/fs_rom
	server/ram_fs
	server/report_rom
}

build $build_components

create_boot_directory

#
# Generate config
#

set lx_ip_addr "10.0.2.55"

set config {
<config verbose="yes">
	<parent-provides>
		<service name="ROM"/>
		<service name="RAM"/>
		<service name="IRQ"/>
		<service name="IO_MEM"/>
		<service name="IO_PORT"/>
		<service name="CAP"/>
		<service name="PD"/>
		<service name="RM"/>
		<service name="CPU"/>
		<service name="LOG"/>
		<service name="SIGNAL"/>
	</parent-provides>
	<default-route>
		<any-service> <parent/> <any-child/> </any-service>
	</default-route>

	<start name="timer">
		<resource name="RAM" quantum="1M"/>
		<provides> <service name="Timer"/> </provides>
	</start> }

append_if $use_platform_driver config {
	<start name="platform_drv">
		<resource name="RAM" quantum="1M" />
		<provides> <service name="Regulator"/>
		           <service name="Platform"/> </provides>
	</start>}

append_if [have_spec gpio] config {
	<start name="gpio_drv">
		<resource name="RAM" quantum="4M"/>
		<provides><service name="Gpio"/></provides>
		<config/>
	</start>}

append_if $use_nic_bridge config {
	<start name="nic_bridge">
		<resource name="RAM" quantum="4M"/>
		<provides><service name="Nic"/></provides>
		<config>}
append_if [expr $use_nic_bridge && [have_spec linux]] config "
			<policy label=\"netserver_genode\" ip_addr=\"$lx_ip_addr\"/>"
append_if $use_nic_bridge config {
		</config>
		<route>
			<service name="Nic"> }
append_if $use_nic_bridge config "
			<child name=\"$network_driver\"/>"
append_if $use_nic_bridge config {
			</service>
			<any-service> <parent/> <any-child/> </any-service>
		</route>
	</start> }

append_if $use_usb_driver config {
	<start name="usb_drv">
		<resource name="RAM" quantum="12M"/>
		<provides>
			<service name="Nic"/>
		</provides>}
append_if $use_usb_driver config "
		<config uhci=\"$use_usb_11\" ehci=\"$use_usb_20\" xhci=\"$use_usb_30\">"
append_if $use_usb_driver config {
			<nic mac="02:00:00:00:01:01" />
		</config>
	</start>}

append_if $use_nic_driver config {
	<start name="nic_drv">
		<resource name="RAM" quantum="4M"/>
		<provides><service name="Nic"/></provides>
	</start>}

append_if $use_wifi_driver config {
	<start name="report_rom">
		<resource name="RAM" quantum="2M"/>
		<provides> <service name="Report" /> <service name="ROM" /> </provides>
		<config> <rom/> </config>
	</start>
	<start name="config_fs">
		<binary name="ram_fs"/>
		<resource name="RAM" quantum="4M"/>
		<provides> <service name="File_system"/> </provides>
		<config>
			<policy label="config_rom" root="/"/>
			<policy label="wifi_drv -> config" root="/" writeable="yes"/>
			<content>
				<inline name="wlan_configuration">}
append_if $use_wifi_driver config "
<selected_network ssid=\"$wifi_ssid\" bssid=\"$wifi_bssid\" protection=\"WPA-PSK\" psk=\"$wifi_psk\"/>"
append_if $use_wifi_driver config {
				</inline>
				<inline name="wpa_supplicant.conf"></inline>
			</content>
		</config>
	</start>
	<start name="config_rom">
		<binary name="fs_rom"/>
		<resource name="RAM" quantum="4M"/>
		<provides><service name="ROM"/></provides>
		<route>
			<service name="File_system"> <child name="config_fs" /> </service>
			<any-service> <parent/> <any-child/> </any-service>
		</route>
	</start>
	<start name="wifi_drv">
		<resource name="RAM" quantum="32M"/>
		<provides> <service name="Nic"/> </provides>
		<config>
			<libc stdout="/dev/log" stderr="/dev/log" rtc="/dev/rtc">
				<vfs>
					<dir name="dev">
						<log/> 
						<jitterentropy name="random"/>
						<jitterentropy name="urandom"/>
					</dir>
					<dir name="config"> <fs label="config"/> </dir>
				</vfs>
			</libc>
		</config>
		<route>
			<service name="Rtc"> <any-child/> </service>
			<service name="File_system"> <child name="config_fs"/> </service>
			<service name="ROM"> <if-arg key="label" value="wlan_configuration" /> <child name="config_rom" /> </service>
			<any-service> <parent/> <any-child /> </any-service>
		</route>
	</start>}

append_if [have_spec acpi] config {
	<start name="acpi">
		<resource name="RAM" quantum="16M" constrain_phys="yes"/>
		<binary name="acpi_drv"/>
		<provides>
			<service name="PCI"/>
			<service name="IRQ" />
		</provides>
		<route>
			<service name="PCI"> <any-child /> </service>
			<any-service> <parent/> <any-child /> </any-service>
		</route>
	</start>}

append_if [expr ![have_spec acpi] && [have_spec pci]] config {
	<start name="pci_drv">
		<resource name="RAM" quantum="3M" constrain_phys="yes"/>
		<provides> <service name="PCI"/> </provides>
	</start> }

append config {
	<start name="netserver_genode">
}
append config "<binary name=\"$netperf_app\"/>"
append config {
		<resource name="RAM" quantum="32M"/>
		<config>
			<arg value="netserver"/>
			<arg value="-D"/>
			<arg value="-4"/>
			<arg value="-f"/>
			<libc tx_buf_size="2M" rx_buf_size="2M"}
append_if [have_spec linux] config " ip_addr=\"$lx_ip_addr\" netmask=\"255.255.255.0\" gateway=\"10.0.2.1\""
append config {
			      stdout="/dev/log" stderr="/dev/log">
				<vfs> <dir name="dev"> <log/> </dir> </vfs>
			</libc>
		</config>}
append_if $use_nic_bridge config {
		<route>
			<service name="Nic"> <child name="nic_bridge"/> </service>
			<any-service> <parent/> <any-child/> </any-service>
		</route>}
append config {
	</start>
</config>
}

install_config $config

#
# Boot modules
#

# generic modules
set boot_modules {
	core init timer
	ld.lib.so libc.lib.so libm.lib.so
}

append                     boot_modules " $netperf_app "
append                     boot_modules " $netperf_stack "
lappend_if $use_nic_bridge boot_modules nic_bridge

# platform-specific modules
lappend_if $use_platform_driver      boot_modules platform_drv
lappend_if [have_spec acpi]          boot_modules acpi_drv
lappend_if [have_spec pci]           boot_modules pci_drv
lappend_if [have_spec gpio]          boot_modules gpio_drv
lappend_if $use_usb_driver           boot_modules usb_drv
lappend_if $use_nic_driver           boot_modules nic_drv
lappend_if [have_spec nova]          boot_modules pci_device_pd

append_if $use_wifi_driver boot_modules {
	ram_fs fs_rom report_rom
	libcrypto.lib.so libssl.lib.so
	wpa_driver_nl80211.lib.so wpa_supplicant.lib.so
	vfs_jitterentropy.lib.so
	wifi.lib.so wifi_drv

	iwlwifi-6000-6.ucode
	iwlwifi-6000g2a-6.ucode
	iwlwifi-6000g2b-6.ucode
	iwlwifi-7260-8.ucode
}

build_boot_image $boot_modules

#
# Execute test case
#

# qemu config
append qemu_args " -m 128 "

append_if [have_spec x86]     qemu_args " -net nic,model=e1000 "
append_if [have_spec lan9118] qemu_args " -net nic,model=lan9118 "

append qemu_args " -net user -redir tcp:12865::12865 -redir tcp:49153::49153 "

run_genode_until {.*and family AF_INET.*} 30
set serial_id [output_spawn_id]

set force_ports ""
if {[have_include "power_on/qemu"]} {
	set ip_addr "localhost"
	set force_ports "-P 49153,49153"
} elseif [have_spec linux] {
	set ip_addr $lx_ip_addr
} else {
	regexp $ip_match_string $output all ip_addr
	puts ""
}

# start netperf client connecting to netperf server running native on Genode
foreach netperf_test $netperf_tests {
	puts "\n---------------------------- $netperf_test -----------------------"

	spawn netperf-$version -H $ip_addr -P 1 -v 2 -t $netperf_test -c -C -- -m $packet_size $force_ports
	set netperf_id $spawn_id

	set spawn_id_list [list $netperf_id $serial_id]

	run_genode_until "Segment" 60 $spawn_id_list

	# get throughput from netperf output
	set throughput [regexp -all -inline {([0-9]+[ ]+){3}[0-9]+\.[0-9]+[ ]+[0-9]+\.[0-9]+} $output]
	set throughput [regexp -all -inline {[0-9]+\.[0-9]+} $throughput]

	# calculate packets per second rate
	set all_bytes [regexp -all -inline {([0-9]+[ ]+){5}} $output]
	set all_bytes [lindex $all_bytes 0 4]

	set elapsed_time [regexp -all -inline {([0-9]+[ ]+){3}[0-9]+\.[0-9]+[ ]+} $output]
	set elapsed_time [lindex $elapsed_time 0 3]
	set packets_second [expr $all_bytes / $packet_size / $elapsed_time]

	puts "\ncalculation: overall bytes / size per packet / time = packets per second"
	puts -nonewline "             $all_bytes Bytes / $packet_size Bytes / $elapsed_time s = "
	puts "[format %8.0f $packets_second] packets/s\n"

	# format output parseable for post proccessing scripts
	puts -nonewline "! PERF: $netperf_test"
	if {$use_nic_bridge} { puts -nonewline "_bridge" }
	if {$use_usb_driver} {
		if {![string compare $use_usb_11 "yes"]} { puts -nonewline "_uhci" }
		if {![string compare $use_usb_20 "yes"]} { puts -nonewline "_ohci" }
		if {![string compare $use_usb_30 "yes"]} { puts -nonewline "_xhci" }
	}
	puts "              [lindex $throughput 1] MBit/s ok"
}
