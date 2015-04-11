<?php   
/* $Id$ */ 
/*      
/* ========================================================================== */
/*      
    emailrelay.inc
    Copyright (C) 2015 Robert Nelson
    All rights reserved.
*/

require_once("config.inc");
require_once("interfaces.inc");
require_once("services.inc");
require_once("service-utils.inc");

function emailrelay_warn($msg)
{
	 log_error("emailrelay: {$msg}");
}

function xtext_encode($input)
{
	$len = strlen($input);
	$output = "";
	for ($index = 0; $index < $len; $index++) {
		$chr = $input[$index];
		if ($chr == '+' || $chr == '=' || $chr < '!' || $chr > '~') {
			$output .= sprintf('+%02X', ord($chr));
		} else {
			$output .= $chr;
		}
	}
	return $output;
}

function emailrelay_resync_config_command()
{
	global $config;
	global $emailrelay_cron_command;

	$emailrelay_config = $config['installedpackages']['emailrelay']['config'][0];
	$enabled = isset($emailrelay_config['enable']);

	$config_file = '/var/etc/emailrelay.conf';
	$auth_file = '/var/etc/emailrelay.auth';
	$valid_file = '/var/etc/emailrelay.valid';

	$configuration = array();
	$configuration[] = 'remote-clients';
	$interface_ips = array();
	foreach (explode(',', $emailrelay_config['interfaces']) as $interface) {
		$interface_ips[] = get_interface_ip($interface);
	}
	$configuration[] = 'interface '.implode(',', $interface_ips);
	$configuration[] = 'client-auth /var/etc/emailrelay.auth';
	if (isset($emailrelay_config['security'])) {
		if ($emailrelay_config['security'] == 'starttls') {
			$configuration[] = 'client-tls';
		} elseif ($emailrelay_config['security'] == 'ssmtp') {
			$configuration[] = 'client-tls-connection';
		}
	}
	$configuration[] = 'forward-to '.$emailrelay_config['server'].':'.$emailrelay_config['port'];
	$configuration[] = 'poll 0';
	if (isset($emailrelay_config['verbose']) && $emailrelay_config['verbose'] == 'on') {
		$configuration[] = 'verbose';
	}
	if (isset($emailrelay_config['debug']) && $emailrelay_config['debug'] == 'on') {
		$configuration[] = 'debug';
	}
	$configuration[] = 'verifier /usr/local/libexec/emailrelay/emailrelay-verify';

	if (file_put_contents($config_file, implode("\n", $configuration)."\n") === FALSE) {
		emailrelay_warn('Couldn\'t write '.$config_file);
		$enabled = False;
	}

	$authorization = 'client '.$emailrelay_config['authentication'].' '.xtext_encode($emailrelay_config['username']).' '.xtext_encode($emailrelay_config['password'])."\n";
	if (file_put_contents($auth_file, $authorization) === False) {
		emailrelay_warn('Couldn\'t write '.$auth_file);
		$enabled = False;
	}

	if (!empty($emailrelay_config['allowed'])) {
		$recipients = base64_decode($emailrelay_config['allowed'])."\n";
	} else {
		$recipients = '';
	}
	
	if (file_put_contents($valid_file, $recipients) === False) {
		emailrelay_warn('Couldn\'t write '.$valid_file);
		$enabled = False;
	}

	if ($enabled) {
		restart_service('emailrelay');
	} else {
		stop_service('emailrelay');
	}
}


function emailrelay_install_command()
{
	$start = <<<EOD
/usr/local/sbin/emailrelay --as-server --pid-file /var/run/emailrelay.pid `egrep -v '^#|^gui-|^ *$' < /var/etc/emailrelay.conf | sed 's/^/--/'`
EOD;
	$stop = <<<EOD
if [ -f /var/run/emailrelay.pid ]; then
	pkill -F /var/run/emailrelay.pid emailrelay
	sleep 5
fi
pkill -9 emailrelay
EOD;
	write_rcfile(array('file' => 'emailrelay', 'start' => $start, 'stop' => $stop));
	
	copy('/usr/local/pkg/emailrelay/emailrelay-verify', '/usr/pbi/emailrelay-'.php_uname("m").'/local/libexec/emailrelay/emailrelay-verify');
	chmod('/usr/pbi/emailrelay-'.php_uname("m").'/local/libexec/emailrelay/emailrelay-verify', 0555);

	stop_service('emailrelay');
	
	$config['installedpackages']['emailrelay']['config'][0]['enable'] = '';

	write_config('Disabled emailrelay until configured');
}


function emailrelay_deinstall_command()
{
	unlink_if_exists('/var/etc/emailrelay.conf');
	unlink_if_exists('/var/etc/emailrelay.auth');
	unlink_if_exists('/var/etc/emailrelay.valid');
}

?>
