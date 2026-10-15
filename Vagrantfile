Vagrant.configure(2) do |config|
  config.vm.box = "debian/trixie64"
  config.vm.box_version = "13.20260519.1"
  config.vm.hostname = 'ft-ping'
  config.vm.define 'ft_ping'
  config.vm.synced_folder '.', '/vagrant', type: 'nfs', disabled: false

  config.vm.provider 'libvirt' do |v|
    v.memory = 4056
    v.cpus = 2
    v.driver = 'kvm'
    v.default_prefix = ''
  end

  config.vm.provision 'shell', inline: <<-SHELL
    set -e
    export DEBIAN_FRONTEND=noninteractive
    apt-get update
    apt-get install -y build-essential wget iptables nftables

    if [ ! -x /usr/local/inetutils-2.0/bin/ping ]; then
      mkdir -p /tmp/inetutils-build
      wget -O /tmp/inetutils-build/inetutils-2.0.tar.gz https://ftp.gnu.org/gnu/inetutils/inetutils-2.0.tar.gz
      tar xzf /tmp/inetutils-build/inetutils-2.0.tar.gz -C /tmp/inetutils-build
      cd /tmp/inetutils-build/inetutils-2.0
      ./configure --prefix=/usr/local/inetutils-2.0
      make
      make install
    fi

    echo 'export PATH="/usr/local/inetutils-2.0/bin:$PATH"' > /etc/profile.d/inetutils.sh
  SHELL
end
