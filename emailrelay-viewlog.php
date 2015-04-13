<?php
/* $Id$ */
/*
	emailrelay-viewlog.php

	Copyright (C) 2015 Robert Nelson <robertn@the-nelsons.org>
	All rights reserved.
*/

require("guiconfig.inc");

if (!$config['installedpackages']['emailrelay']['config'][0])
	Header("Location: /pkg_edit.php?xml=emailrelay.xml&id=0");

$emailrelay_logfile = "{$g['varlog_path']}/emailrelay.log";

$nentries = $config['syslog']['nentries'];
if (!$nentries)
	$nentries = 50;

if ($_POST['clear'])
	clear_log_file($emailrelay_logfile);

if ($_GET['filtertext'])
	$filtertext = htmlspecialchars($_GET['filtertext']);

if ($_POST['filtertext'])
	$filtertext = htmlspecialchars($_POST['filtertext']);

if ($filtertext)
	$filtertextmeta="?filtertext=$filtertext";

$pgtitle = "E-MailRelay: Log";
include("head.inc");

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
	<?php include("fbegin.inc"); ?>
	<table width="100%" border="0" cellpadding="0" cellspacing="0">
		<tr>
			<td>
				<?php
					$tab_array = array();
					$tab_array[] = array(gettext("Settings"), false, "/pkg_edit.php?xml=emailrelay.xml&id=0");
					$tab_array[] = array(gettext("Log"), true, "/emailrelay-viewlog.php");
					display_top_tabs($tab_array);
				?>
			</td>
		</tr>
		<tr>
			<td>
				<div id="mainarea">
					<table class="tabcont" width="100%" border="0" cellspacing="0" cellpadding="0" summary="main area">
						<tr>
							<td colspan="2" class="listtopic"><?php printf(gettext("Last %s E-MailRelay log entries"),$nentries); ?></td>
						</tr>
						<?php
							if ($filtertext)
								dump_clog($emailrelay_logfile, $nentries, true, array("$filtertext"));
							else
								dump_clog($emailrelay_logfile, $nentries);
						?>
						<tr>
							<td align="left" valign="top">
								<form id="filterform" name="filterform" action="emailrelay-viewlog.php" method="post" style="margin-top: 14px;">
									<input id="submit" name="clear" type="submit" class="formbtn" value="<?=gettext("Clear log");?>" />
								</form>
							</td>
							<td align="right" valign="top" >
								<form id="clearform" name="clearform" action="emailrelay-viewlog.php" method="post" style="margin-top: 14px;">
									<input id="filtertext" name="filtertext" value="<?=$filtertext;?>" />
									<input id="filtersubmit" name="filtersubmit" type="submit" class="formbtn" value="<?=gettext("Filter");?>" />
								</form>
							</td>
						</tr>
					</table>
				</div>
			</td>
		</tr>
	</table>
<?php include("fend.inc"); ?>
</body>
</html>
