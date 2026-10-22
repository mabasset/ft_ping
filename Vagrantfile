Vagrant.configure(2) do |config|
  config.vm.box = "bento/debian-13"
  config.vm.box_version = "202510.26.0"
  config.vm.hostname = 'ft-ping'
  config.vm.define 'ft_ping'
  config.vm.synced_folder '.', '/vagrant'

  config.vm.provider 'virtualbox' do |v|
    v.name = 'ft_ping'
    v.memory = 4096
    v.cpus = 2
  end

  config.vm.provision 'shell', inline: <<-SHELL
    set -e
    export DEBIAN_FRONTEND=noninteractive
    apt-get update
    apt-get install -y build-essential wget iptables nftables tmux

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
