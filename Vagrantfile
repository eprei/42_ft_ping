vm_dev = "dev"

Vagrant.configure("2") do |config|
    config.vm.box = "ubuntu/jammy64"
    config.vm.synced_folder ".", "/vagrant", disabled: true

    config.vm.define vm_dev do |control|
        control.vm.hostname = vm_dev
        control.vm.provider "virtualbox" do |vb|
            vb.memory = 12288
            vb.cpus = 2
            vb.name = vm_dev
            vb.linked_clone = true
            vb.gui = true
            vb.customize ["modifyvm", :id, "--vram", "128"]
            vb.customize ["modifyvm", :id, "--graphicscontroller", "vmsvga"]
            vb.customize ["modifyvm", :id, "--clipboard", "bidirectional"]
            vb.customize ["setextradata", :id, "CustomVideoMode1", "1920x1080x24"]
        end

        control.vm.provision "shell", inline: <<-SHELL
            export DEBIAN_FRONTEND=noninteractive
            echo "wireshark-common wireshark-common/install-setuid boolean true" | debconf-set-selections

            apt-get update
            apt-get upgrade -y

            # Dev tools
            apt-get install -y build-essential \
            cmake \
            git \
            gdb \

            # Minimal graphic environment and CLion
            apt-get install -y --no-install-recommends \
                ubuntu-desktop-minimal \
                lightdm \
                snapd

            # Set up autologin for vagrant user
            mkdir -p /etc/lightdm/lightdm.conf.d
            echo "[Seat:*]
autologin-user=vagrant
autologin-user-timeout=0" > /etc/lightdm/lightdm.conf.d/autologin.conf

            snap install clion --classic

            # Automatic graphic mode when starting the VM
            systemctl set-default graphical.target

            # Guest Additions para mejor rendimiento
            apt-get install -y virtualbox-guest-x11

            reboot
        SHELL
    end
end