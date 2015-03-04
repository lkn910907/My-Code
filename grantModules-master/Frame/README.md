---------->The Popurse

    Why I write this little module ? It is not for tricks, although it looks like a little trick.
Do you know the one of the method of I/O Virtualization---Single Root IOV, you can get knowledges about SR_IOV in website like http://www.pcisig.com/specifications/iov/ or http://www.intel.com/content/www/us/en/pci-express/pci-sig-sr-iov-primer-sr-iov-technology-paper.html
  We have a little team ,that attend to make a NIC Card with the SR-IOV feature,but it difficult for us to implenment the MAILBOX mechanisam for PF2VF communications.So we intend to make the communication with software.like the grant table,shared page and event channel or IO ring,or Xenstore.
    As you already know, the SR-IOV can be implemented only in the HVMs ,so we must make sure the module can be implemented in the HVM successfully, Which means we should configure PVHVM in th HVM fully virtualization guestss.
    After all, you can treat this like a little trick to lean about the use or mechanisam ofeveent channle, grant table and xenstore to understand the machanisam of paravirtualization of xen.


   the front file is for the domu-use
   the backend file is for the dom0-use
   the test file is something I use to verify the functions when I code,you can ignore this!
