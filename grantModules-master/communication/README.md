---------->The Popurse

    Why I write this little module ? It is not for tricks, although it looks like a little trick.
Do you know the one of the method of I/O Virtualization---Single Root IOV, you can get knowledges about SR_IOV in website like http://www.pcisig.com/specifications/iov/ or http://www.intel.com/content/www/us/en/pci-express/pci-sig-sr-iov-primer-sr-iov-technology-paper.html
  We have a little team ,that attend to make a NIC Card with the SR-IOV feature,but it difficult for us to implenment the MAILBOX mechanisam for PF2VF communications.So we intend to make the communication with software.like the grant table,shared page and event channel or IO ring,or Xenstore.
    As you already know, the SR-IOV can be implemented only in the HVMs ,so we must make sure the module can be implemented in the HVM successfully.
    After all, you can treat this like a little trick to lean about the use or mechanisam of grant table.
