# colo-manage-tools 
Manage tool for COLO-based fault-tolerant VM.

# how to install

base on : QEMU-COLO

requirements: libzmq-4.3.5

``` shell
cd $PROJECT_DIR
mkdir build
cd build
make -j8
sudo make install
cd ../bin
```

# how to use

## run colod
``` shell
cd $PROJECT_DIR
cd bin
sudo ./colod
```

## prepare config file

colo-manage-tool use config file to configure connection to peer colo node.
in config file, local status and peer status are necessary.

``` yaml
colo:
  local_status: primary
  host_ip: 192.168.10.2
  host_user: ubuntu
  host_file_path: /home/ubuntu/config/colo_manage_tools/
  peer_ip: 192.168.10.3
  peer_user: ubuntu
  peer_file_path: /home/ubuntu/config/colo_manage_tools/
```

## prepare domain config file

Domain config file describes a COLO-VM's configuration in yaml format, and the content is similiar to libvirt domain xml.

Here is an example.
``` yaml
name: test
memory: 2048
qemu: /usr/local/bin/qemu-system-x86_64
vcpu: 4
os:
  arch: x86_64
  machine: pc-i440fx-4.2
  type: hvm
cpu: host

disk:
  driver: qcow2
  size: 307200 
  path: /home/ubuntu/vmtest/ubuntu18.qcow2
  hidden_path: /home/ubuntu/vmtest/secondary-hidden.qcow2
  active_path: /home/ubuntu/vmtest/secondary-active.qcow2

net:
  id: hn0
  br: virbr0
  helper: /usr/local/libexec/qemu-bridge-helper
  mac: 00:0c:29:5e:72:33
 
colo:
  ports: [9003,9004,9005,9006,9007]
  pri:
    telnet_port: 4444
    mirror_port: 9003
    cmp0_port: 9001
    cmp1_port: 9004
    cmp_out_port: 9005
    
  sec:
    telnet_port: 4444
    nbd_port: 9999
    migrate_port: 8888
    red0_port: 9003
    red1_port: 9004
```

## start a colo vm

``` shell
cd $PROJECT_DIR
cd bin
# connect to peer node
./colo_manage_tools -t connect-peer -c $CONFIG_FILE
./colo_manage_tools -t connect-status

# define a colo enabled domain and start it
./colo_manage_tools -t define -v $DOMAIN_FILE
./colo_manage_tools -t colo-enable -d $DOMAIN_NAME
./colo_manage_tools -t start -d $DOMAIN_NAME --colo
./colo_manage_tools -t vm-status -d $DOMAIN_NAME 

# set params and do failover
./colo_manage_tools -t set-params -d $DOMAIN_NAME --checkpoint-time 3000
./colo_manage_tools -t do-failover -d $DOMAIN_NAME


```


# available commands

## colo-manage-tools usage
```
usage: ../bin/colo_manage_tools --command-type=string [options] ... 
options:
  -t, --command-type       command type (string)
  -v, --vm-file            vm definition file (string [=])
  -c, --config-file        configuration file (string [=])
  -d, --domain             domain name (string [=])
  -q, --qmp-command        qmp-command (string [=])
      --all                show all the vms
      --colo               start vm in colo mode
      --checkpoint-time    checkpoint time (int [=1000])
      --compare-timeout    compare timeout (int [=1000])
      --max-queue-size     max queue size (int [=65535])
  -r, --restart-colod      restart colod
  -?, --help               print this message
```

## all management commands

### connect-peer
**NAME**
    
    connect-peer - connect peer colod

**SYNOPSIS**
    
    colo_manage_tools -t connect-peer [(-c|--config-file) <string>]

**DESCRIPTION**

    connect peer colod and record it

**OPTIONS**
    
    --config-file <string>   colo config file

### connect-status
**NAME**
    
    connect-status - show connect status

**SYNOPSIS**
    
    colo_manage_tools -t connect-status

**DESCRIPTION**
    
    show current connect-status

### define
**NAME**
    
    define - define a domain

**SYNOPSIS**
    
    colo_manage_tools -t define [(-v|--vm-file) <string>]

**DESCRIPTION**
    
    define a COLO-VM with given vm-file

**OPTIONS**
    
    --vm-file <string>   colo domain config file

### undefine
**NAME**
    
    undefine - undefine a domain

**SYNOPSIS**
    
    colo_manage_tools -t undefine [(-d|--domain) <string>]

**DESCRIPTION**
    
    define a COLO-VM

**OPTIONS**
    
    --domain <string>   colo domain name

### list
**NAME**
    
    list - list all the domains

**SYNOPSIS**
    
    colo_manage_tools -t list [--all]

**DESCRIPTION**
    
    list all the domains

**OPTIONS**
    
    --all   show all the domains(include the shutoff domain)

### start
**NAME**
    
    start - start a domain

**SYNOPSIS**
    
    colo_manage_tools -t start [(-d|--domain) <string>] [--colo]

**DESCRIPTION**
    
    start a domain, in native mode or colo-mode

**OPTIONS**
    
    --domain <string>   colo domain name
    --colo              start domain with colo-enabled

### destroy
**NAME**
    
    destroy - destroy a domain

**SYNOPSIS**
    
    colo_manage_tools -t destroy [(-d|--domain) <string>]

**DESCRIPTION**
    
    destroy a running domain

**OPTIONS**
    
    --domain <string>   colo domain name

### colo-enable
**NAME**
    
    colo-enable - make a domain colo-enabled

**SYNOPSIS**
    
    colo_manage_tools -t colo-enable [(-d|--domain) <string>]

**DESCRIPTION**
    
    colo-enable a shutoff domain, transfer its config-file and disk-file to peer node

**OPTIONS**
    
    --domain <string>   colo domain name

### colo-disable
**NAME**
    
    colo-disable - make a domain colo-disabled

**SYNOPSIS**
    
    colo_manage_tools -t colo-disable [(-d|--domain) <string>]

**DESCRIPTION**
    
    colo-disable a shutoff domain, and this domain can not start in colo-mode

**OPTIONS**

    --domain <string>   colo domain name

### vm-status
**NAME**
    
    vm-status - show a domain detailed status

**SYNOPSIS**
    
    colo_manage_tools -t vm-status [(-d|--domain) <string>]

**DESCRIPTION**
    
    show a domain detailed status

**OPTIONS**
    
    --domain <string>   colo domain name

### set-params
**NAME**
    
    set-params - set colo-enabled domains running params

**SYNOPSIS**
    
    colo_manage_tools -t set-params [(-d|--domain) <string>] [--$PROPERTY <int>]

**DESCRIPTION**
    
    show a domain detailed status

**OPTIONS**
    
    --domain <string>    colo domain name
    --checkpoint-time    checkpoint time (int [=1000])
    --compare-timeout    compare timeout (int [=1000])
    --max-queue-size     max queue size (int [=65535])

### do-failover
**NAME**
    
    do-failover - do failover for a colo-enabled domain

**SYNOPSIS**
    
    colo_manage_tools -t do-failover [(-d|--domain) <string>]

**DESCRIPTION**
    
    do failover for a colo-enabled running domain

**OPTIONS**
    
    --domain <string>   colo domain name



