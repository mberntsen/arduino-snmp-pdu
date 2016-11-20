# arduino-snmp-pdu

sources:
https://tobinsramblings.wordpress.com/2011/05/03/snmp-tutorial-apc-pdus/

usage requirements:
1. $ sudo apt install snmp-mibs-downloader
2. put the files from the following repository in the right place (~/.snmp)
   git://github.com/joshuatobin/dotfiles.git

usage examples:

$ snmpwalk -v 1 -c public 192.168.1.200
RFC1213-MIB::sysDescr = STRING: "Agentuino, a light-weight SNMP Agent."
RFC1213-MIB::sysObjectID = Wrong Type (should be OBJECT IDENTIFIER): STRING: "1.3.6.1.3.2009.0"
RFC1213-MIB::sysUpTime = Timeticks: (0) 0:00:00.00
RFC1213-MIB::sysContact = STRING: "Eric Gionet"
RFC1213-MIB::sysName = STRING: "Agentuino"
RFC1213-MIB::sysLocation = STRING: "Nova Scotia, CA"
RFC1213-MIB::sysServices = INTEGER: 7
End of MIB

$ snmpwalk -v 1 -c public 192.168.1.200 enterprises
PowerNet-MIB::sPDUOutletCtl.1 = INTEGER: outletOn(1)
PowerNet-MIB::sPDUOutletCtl.2 = INTEGER: outletOn(1)
PowerNet-MIB::sPDUOutletCtl.3 = INTEGER: outletOff(2)
PowerNet-MIB::sPDUOutletCtl.4 = INTEGER: outletOff(2)
PowerNet-MIB::sPDUOutletCtl.5 = INTEGER: outletOff(2)
PowerNet-MIB::sPDUOutletCtl.6 = INTEGER: outletOff(2)
PowerNet-MIB::sPDUOutletCtl.7 = INTEGER: outletOff(2)
PowerNet-MIB::sPDUOutletCtl.8 = INTEGER: outletOff(2)
End of MIB

$ snmpset -v 1 -c private 192.168.1.200 PowerNet-MIB::sPDUOutletCtl.1 i 1
PowerNet-MIB::sPDUOutletCtl.1 = INTEGER: outletOn(1)

$ snmpset -v 1 -c private 192.168.1.200 PowerNet-MIB::sPDUOutletCtl.2 i 1
PowerNet-MIB::sPDUOutletCtl.2 = INTEGER: outletOn(1)

$ snmpset -v 1 -c private 192.168.1.200 PowerNet-MIB::sPDUOutletCtl.1 i 2
PowerNet-MIB::sPDUOutletCtl.1 = INTEGER: outletOff(2)

