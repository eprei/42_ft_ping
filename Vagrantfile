machine_name = "DevEnv"

Vagrant.configure("2") do |config|
    config.vm.box = "ubuntu/jammy64"
    config.vm.synced_folder '.', '/home/vagrant/ft_ping'

    config.vm.define machine_name do |control|
        control.vm.hostname = machine_name

        control.vm.provider "virtualbox" do |vb|
            vb.memory = 12288
            vb.cpus = 3
            vb.name = machine_name
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
                g++

            # Minimal graphic environment
            apt-get install -y --no-install-recommends \
                ubuntu-desktop-minimal \
                lightdm

            # Set up autologin for vagrant user
            mkdir -p /etc/lightdm/lightdm.conf.d
            echo "[Seat:*]
autologin-user=vagrant
autologin-user-timeout=0" > /etc/lightdm/lightdm.conf.d/autologin.conf

            # Install snapd and CLion
            apt-get install -y snapd
            snap install clion --classic

            # Automatic graphic mode
            systemctl set-default graphical.target

            reboot
        SHELL
    end
end