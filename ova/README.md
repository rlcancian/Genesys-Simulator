# OVA - GenESyS Simulator Virtual Environment

## Motivation and Objective

To run the simulator in environments where the system has restrictions, such as lack of required permissions, inconsistent environments across machines, or missing dependencies, a virtualization environment was developed through the use of OVAs.

An OVA (Open Virtual Appliance) is a virtual machine distribution format that encapsulates the disk, configuration, and metadata into a single file. This allows the same environment to be imported and executed on different machines, ensuring reproducibility.

In this project, the OVA is used to provide a complete and preconfigured environment for running the GenESyS Simulator.

The choice of the base image followed criteria of stability, compatibility, and size. Debian (netinst) with the LXDE graphical environment and LightDM was chosen because it provides a lightweight yet sufficiently functional combination for laboratory use. For this reason, the environment configuration scripts were developed for Debian-based systems.

---

## How to Build a New OVA

The process consists of creating a base VM, installing the system, and applying the environment configuration script.

After installing Debian (netinst) without a graphical interface in VirtualBox, the minimum setup script must be obtained and executed.

The complete guide for building a new OVA can be found at `documentation/ova/Construction_of_a_New_OVA.pdf` from the root folder of this repository.

---

## User Guide

Contains instructions intended for end users of the OVA, including importing the virtual machine into VirtualBox, login credentials, usage of the GenESyS Simulator, usage of QtCreator, Guest Additions configuration, terminal usage, and switching development branches.

It can be found at `documentation/ova/User_Guide.pdf` from the root folder of this repository.

---

## Developer Guide

Contains technical information intended for the maintenance and evolution of the OVA, including environment architecture, installed tools, operation of the automation scripts (`min_setup_qtcreator.sh`, `init.sh`, `update.sh`, and `install_guest_add.sh`), release management, incremental OVA updates, web service operation, and the location of the main system files.

It can be found at `documentation/ova/Developer_Guide.pdf` from the root folder of this repository.