#!/usr/bin/php5 -q
<?php

require_once("../../gosa-core/include/class_socketClient.inc");
error_reporting(E_ALL);

$zahl= 1;

for($count = 1; $count <= $zahl; $count++)
{

  $sock = new Socket_Client("127.0.0.1","20081",TRUE,5);
  $sock->setEncryptionKey("secret-gosa-password");
  #$sock = new Socket_Client("10.89.1.42","20081",TRUE,5);
  #$sock->setEncryptionKey("secret-gosa-password");

  if($sock->connected()){

    /* Prepare a hunge bunch of data to be send */
    # jobdb add
    #$data = "<xml> <header>gosa_network_completition</header> <source>GOSA</source><target>GOSA</target><hostname>ws-muc-2</hostname></xml>";
    #$data = "<xml> <header>job_sayHello</header> <source>10.89.1.155:20083</source><target>00:01:6c:9d:b9:fa</target><mac>00:1B:77:04:8A:6C</mac> <timestamp>20130102133908</timestamp> </xml>";
    #$data = "<xml> <header>job_ping</header> <source>GOSA</source> <target>00:01:6c:9d:b9:fa</target> <macaddress>00:01:6c:9d:b9:fa</macaddress><timestamp>20130101000000</timestamp> </xml>";
  
    # jobdb delete
    #$data = "<xml> <header>gosa_delete_jobdb_entry</header> <source>GOSA</source> <target>GOSA</target> <where><clause><phrase><id>3</id></phrase></clause></where></xml>";

    # smbhash
    #$data = "<xml> <header>gosa_gen_smb_hash</header> <source>GOSA</source><target>GOSA</target><password>tester</password></xml>";

    # Reload ldap config
    #$data = "<xml> <header>gosa_trigger_reload_ldap_config</header> <source>GOSA</source><target>00:01:6c:9d:b9:fa</target></xml>";

    # jobdb update  
    #$data = "<xml> <header>gosa_update_status_jobdb_entry</header> <source>GOSA</source> <target>GOSA</target> <where><clause><phrase> <id>1</id></phrase></clause></where> <update><timestamp>19700101000000</timestamp></update></xml>";
    #$data = "<xml> <header>gosa_update_status_jobdb_entry</header> <source>GOSA</source><target>GOSA</target><where><clause><phrase> <macaddress>00:01:6c:9d:b9:fa</macaddress></phrase></clause> </where> <update><status>processing</status> <result>update</result></update></xml>";

    # jobdb query
    #$data = "<xml><header>gosa_query_jobdb</header><source>GOSA</source> <target>GOSA</target><where><clause><connector>and</connector><phrase><operator>gt</operator><ROWID>0</ROWID></phrase><phrase><operator>le</operator><ROWID>5</ROWID></phrase></clause></where></xml>";
    #$data = "<xml><header>gosa_query_jobdb</header><source>GOSA</source> <target>GOSA</target> <where> <clause> <phrase> <operator>like</operator> <timestamp>%0102%</timestamp> </phrase> </clause> </where> </xml>";
    #$data= "<xml><header>gosa_query_jobdb</header><source>GOSA</source> <target>GOSA</target><where><clause><phrase><headertag>ping</headertag></phrase></clause></where><limit><from>0</from><to>3</to></limit></xml>";
    #$data= "<xml><header>gosa_query_jobdb</header><source>GOSA</source> <target>GOSA</target><where><clause><phrase><HEADERTAG>trigger_action_reinstall</HEADERTAG></phrase></clause></where><limit><from>0</from><to>25</to></limit><orderby>timestamp DESC</orderby></xml>";
    #$data= "<xml><header>gosa_query_jobdb</header><source>GOSA</source> <target>GOSA</target></xml>";
    #$data= "<xml><header>gosa_query_fai_server</header><source>GOSA</source> <target>GOSA</target></xml>";
    #$data= "<xml><header>gosa_query_fai_release</header><source>GOSA</source> <target>GOSA</target></xml>";
    #$data= "<xml><header>gosa_query_packages_list</header><source>GOSA</source> <target>GOSA</target></xml>";


    # jobdb count
    #$data = "<xml> <header>gosa_count_jobdb</header><source>GOSA</source> <target>GOSA</target></xml>";
    #$data = "<xml> <header>gosa_count_fai_server</header><source>GOSA</source> <target>GOSA</target></xml>";
    #$data = "<xml> <header>gosa_count_fai_release</header><source>GOSA</source> <target>GOSA</target></xml>";

    # jobdb clear
    #$data = "<xml> <header>gosa_clear_jobdb</header> <source>GOSA</source> <target>GOSA</target></xml>";

    # set gosa-si-client to 'activated'
    #$data = "<xml> <header>job_set_activated_for_installation</header> <target>10.89.1.31:20083</target> <source>GOSA</source> <macaddress>00:01:6c:9d:b9:fa</macaddress><timestamp>22220101000000</timestamp></xml>";


    # trigger jobs at client
    #$data = "<xml> <header>gosa_trigger_goto_settings_reload</header> <target>00:01:6c:9d:b9:fa</target> <source>GOSA</source> </xml>";
    #$data = "<xml> <header>gosa_detect_hardware</header> <target>10.89.1.31:20083</target> <source>GOSA</source> </xml>";
    #$data = "<xml> <header>gosa_new_key_for_client</header> <target>00:01:6c:9d:b9:fa</target> <source>10.89.1.31:20081</source> </xml>";
    #$data = "<xml> <header>job_trigger_action_wake</header> <target>00:01:6c:9d:b9:fa</target> <source>GOSA</source> <timestamp>19700101000000</timestamp></xml>";
    #$data = "<xml> <header>gosa_trigger_action_faireboot</header> <target>00:01:6c:9d:b9:fa</target> <source>GOSA</source> </xml>";
    #$data = "<xml> <header>gosa_trigger_action_reboot</header> <target>00:01:6c:9d:b9:fa</target> <source>GOSA</source> </xml>";
    #$data = "<xml> <header>gosa_trigger_action_halt</header> <target>00:01:6c:9d:b9:fa</target> <source>GOSA</source> </xml>";
    #$data = "<xml> <header>job_trigger_action_reinstall</header> <source>GOSA</source> <target>00:01:6c:9d:b9:fa</target> <macaddress>00:01:6c:9d:b9:fa</macaddress> <timestamp>19700101000000</timestamp> </xml>";
    #$data = "<xml> <header>job_trigger_action_instant_update</header> <target>00:01:6c:9d:b9:fa</target> <source>GOSA</source> <timestamp>19700101000000</timestamp> </xml>";
    #$data = "<xml> <header>gosa_new_ping</header> <target>00:01:6c:9d:b9:fa</target> <source>GOSA</source> </xml>";

    # get_login_usr_for_client
    #$data = "<xml> <header>gosa_get_login_usr_for_client</header> <target>GOSA</target> <source>GOSA</source> <client>00:01:6c:9d:b9:fa</client></xml>";

    # get_client_for_login_usr
    #$data = "<xml> <header>gosa_get_client_for_login_usr</header> <target>GOSA</target> <source>GOSA</source> <usr>rettenbe</usr></xml>";

    # List all si-server providing opsi
    #$data = "<xml> <header>gosa_get_hosts_with_module</header> <source>GOSA</source> <target>10.89.1.31:20081</target> <module_name>mailqueue_com</module_name> </xml>";
    $data = "<xml> <header>gosa_get_hosts_with_module</header> <source>GOSA</source> <target>linux-cl-2:20081</target> <module_name>mailqueue_com</module_name> </xml>";

    # Send messages to a user and displayed message via konch
    #$data = "<xml> <header>gosa_send_user_msg</header> <target>GOSA</target> <source>GOSA</source> <subject>".base64_encode("eine wichtige nachricht")."</subject> <from>admin</from>  <user>polle</user> <user>harald</user> <delivery_time>20130101235959</delivery_time> <message>".base64_encode("kaffeepause")."</message> </xml>"; 
    #$data = "<xml> <header>gosa_send_user_msg</header> <target>10.89.1.30:20081</target> <source>GOSA</source> <subject>".base64_encode("eine wichtige nachricht")."</subject> <from>admin</from> <user>polle</user> <user>harald</user> <delivery_time>20130101235959</delivery_time> <message>".base64_encode("kaffeepause")."</message> </xml>";
    #$data = "<xml> <header>gosa_send_user_msg</header> <target>10.89.1.30:20081</target> <source>GOSA</source> <subject>".base64_encode("eine wichtige nachricht")."</subject> <from>admin</from> <user>rettenbe</user> <delivery_time>20130101235959</delivery_time> <message>".base64_encode("kaffeepause")."</message> </xml>";
    
    #$data = "<xml> <header>gosa_set_last_system</header> <target>10.89.1.31:20082</target> <source>GOSA</source> <mac_address>00:01:6c:9d:b9:fa</mac_address> <last_system>1.2.3.4</last_system> <last_system_login>20081212000000</last_system_login> </xml>";

    ##################
    # recreate fai dbs
    #$data = "<xml> <header>gosa_recreate_fai_server_db</header> <target>GOSA</target> <source>GOSA</source></xml>"; 
    #$data = "<xml> <header>gosa_recreate_fai_release_db</header> <target>GOSA</target> <source>GOSA</source></xml>"; 
    #$data = "<xml> <header>gosa_recreate_packages_list_db</header> <target>GOSA</target> <source>GOSA</source></xml>"; 


    ################
    # logHandling.pm
    # all date and mac parameter accept regular expression as input unless other instructions are given
    # show_log_by_mac, show_log_by_date, show_log_by_date_and_mac, show_log_files_by_date_and_mac, 
    # get_log_file_by_date_and_mac, delete_log_by_date_and_mac, get_recent_log_by_mac
    #$data = "<xml> <header>gosa_show_log_by_mac</header> <target>GOSA</target> <source>GOSA</source> <mac>00:01:6C:9D:B9:FA</mac> <mac>00:01:6c:9d:b9:fb</mac> </xml>"; 
    #$data = "<xml> <header>gosa_show_log_by_date</header> <target>GOSA</target> <source>GOSA</source> <date>20080313</date> <date>20080323</date> </xml>"; 
    #$data = "<xml> <header>gosa_show_log_by_date_and_mac</header> <target>GOSA</target> <source>GOSA</source> <date>200803</date> <mac>00:01:6c:9d:b9:FA</mac> </xml>"; 
    #$data = "<xml> <header>gosa_delete_log_by_date_and_mac</header> <target>GOSA</target> <source>GOSA</source>  <mac>00:01:6c:9d:b9:fa</mac></xml>"; 
    #$data = "<xml> <header>gosa_get_recent_log_by_mac</header> <target>GOSA</target> <source>GOSA</source> <mac>00:01:6c:9d:b9:fa</mac></xml>"; 
    # exact date and mac are required as input
    #$data = "<xml> <header>gosa_show_log_files_by_date_and_mac</header> <target>GOSA</target> <source>GOSA</source> <date>install_20080311_090900</date> <mac>00:01:6c:9d:b9:fa</mac> </xml>"; 
    #$data = "<xml> <header>gosa_get_log_file_by_date_and_mac</header> <target>GOSA</target> <source>GOSA</source> <date>install_20080311_090900</date> <mac>00:01:6c:9d:b9:fa</mac> <log_file>boot.log</log_file></xml>"; 

    #########
    # Kerberos test query
    #$data = "<xml> <header>gosa_krb5_create_principal</header> <target>00:01:6c:9d:aa:16</target> <principal>horst@WIRECARD.SYS</principal><source>GOSA</source><max_life>666</max_life></xml>"; 
    #$data = "<xml> <header>gosa_krb5_modify_principal</header> <target>00:01:6c:9d:b9:fa</target> <principal>horst@WIRECARD.SYS</principal><source>GOSA</source><max_life>666</max_life></xml>"; 

    #$data = "<xml><header>gosa_query_fai_server</header><source>GOSA</source> <target>10.89.1.131:20081</target></xml>";
    #$data = "<xml> <header>gosa_ping</header> <target>00:01:6c:9d:aa:16</target> <source>GOSA</source> </xml>";
    #$data = "<xml> <header>gosa_ping</header> <target>00:01:6c:9d:b9:fb</target> <source>GOSA</source> </xml>";
    #$data = "<xml> <header>gosa_get_dak_keyring</header> <target>GOSA</target> <source>GOSA</source> </xml>";
    #$data = "<xml> <header>job_ping</header> <source>GOSA</source> <target>00:0c:29:02:e5:4d</target> <macaddress>00:0c:29:02:e5:4d</macaddress><timestamp>29700101000000</timestamp> </xml>";

    #$data = "<xml> <header>gosa_network_completition</header> <source>GOSA</source> <target>GOSA</target> <hostname>localhost</hostname> </xml>";

    ####################################################################
    # Opsi testing

    # Get all netboot products
    #$data = "<xml> <header>gosa_opsi_get_netboot_products</header> <source>GOSA</source> <target>GOSA</target> </xml>";

    # Get netboot product for specific host
    #$data = "<xml> <header>gosa_opsi_get_netboot_products</header> <source>GOSA</source> <target>GOSA</target> <hostId>linux-cl-2.intranet.gonicus.de</hostId></xml>";

    # Get all localboot products
    #$data = "<xml> <header>gosa_opsi_get_local_products</header> <source>GOSA</source> <target>GOSA</target> </xml>";
    
    # Get localboot product for specific host
    #$data = "<xml> <header>gosa_opsi_get_local_products</header> <source>GOSA</source> <target>GOSA</target> <hostId>linux-cl-2.intranet.gonicus.de</hostId></xml>";

    # Get product properties - global
    #$data = "<xml> <header>gosa_opsi_get_product_properties</header> <source>GOSA</source> <target>GOSA</target> <productId>xpconfig</productId></xml>";

    # Get product properties - per host
    #$data = "<xml> <header>gosa_opsi_get_product_properties</header> <source>GOSA</source> <target>GOSA</target> <productId>firefox</productId> <hostId>linux-cl-2.intranet.gonicus.de</hostId> </xml>";

    # Set product properties - global
    #$data = "<xml> <header>gosa_opsi_set_product_properties</header> <source>GOSA</source> <target>00:01:6c:9d:b9:fa</target> <productId>javavm</productId> <item><name>askbeforeinst</name><value>false</value></item></xml>";

    # Set product properties - per host
    #$data = "<xml> <header>gosa_opsi_set_product_properties</header> <source>GOSA</source> <target>00:01:6c:9d:b9:fa</target> <hostId>linux-cl-2.intranet.gonicus.de</hostId> <productId>firefox</productId> <item> <name>askbeforeinst</name> <value>false</value> </item> </xml>";

    # Get hardware inventory
    #$data = "<xml> <header>gosa_opsi_get_client_hardware</header> <source>GOSA</source> <target>GOSA</target> <hostId>linux-cl-2.intranet.gonicus.de</hostId> </xml>";
    #$data = "<xml> <header>gosa_opsi_get_client_hardware</header> <source>GOSA</source> <target>GOSA</target> <hostId>der_neue.intranet.gonicus.de</hostId> </xml>";
    
    # Get software inventory
    #$data = "<xml> <header>gosa_opsi_get_client_software</header> <source>GOSA</source> <target>GOSA</target> <hostId>linux-cl-2.intranet.gonicus.de</hostId> </xml>";

    # List Opsi clients
    #$data = "<xml> <header>gosa_opsi_list_clients</header> <source>GOSA</source> <target>GOSA</target> </xml>";

    # Delete Opsi client
    #$data = "<xml> <header>gosa_opsi_del_client</header> <source>GOSA</source> <target>00:01:6c:9d:b9:fa</target> <hostId>linux-cl-2.intranet.gonicus.de</hostId></xml>";

    # Install Opsi client
    # Please do always use 'job_...' and never 'gosa_...' otherways the job will never appear in job queue
    #$data = "<xml> <header>job_opsi_install_client</header> <source>GOSA</source> <target>00:01:6c:9d:b9:fa</target> <hostId>linux-cl-2.intranet.gonicus.de</hostId> <macaddress>00:11:25:4b:8c:e5</macaddress> </xml>";

    # Add Opsi client
    #$data = "<xml> <header>gosa_opsi_add_client</header> <source>GOSA</source> <target>00:01:6c:9d:b9:fa</target> <hostId>linux-cl-2.intranet.gonicus.de</hostId> <macaddress>00:11:25:4b:8c:e5</macaddress> <description>Test halt</description> <ip>1.2.3.4</ip> <notes>Im a note</notes> </xml>";

    # Add product to Opsi client
    #$data = "<xml> <header>gosa_opsi_add_product_to_client</header> <source>GOSA</source> <target>00:01:6c:9d:b9:fa</target> <macaddress>00:11:25:4b:8c:e5</macaddress> <hostId>linux-cl-2.intranet.gonicus.de</hostId> <productId>winxppro</productId> </xml>";
    #$data = "<xml> <header>gosa_opsi_add_product_to_client</header> <source>GOSA</source> <target>00:01:6c:9d:b9:fa</target> <macaddress>00:11:25:4b:8c:e5</macaddress> <hostId>linux-cl-2.intranet.gonicus.de</hostId> <productId>preloginloader</productId> </xml>";
    #$data = "<xml> <header>gosa_opsi_add_product_to_client</header> <source>GOSA</source> <target>00:01:6c:9d:b9:fa</target> <macaddress>00:11:25:4b:8c:e5</macaddress> <hostId>linux-cl-2.intranet.gonicus.de</hostId> <productId>opsi-adminutils</productId> </xml>";
    #$data = "<xml> <header>gosa_opsi_add_product_to_client</header> <source>GOSA</source> <target>00:01:6c:9d:b9:fa</target> <macaddress>00:11:25:4b:8c:e5</macaddress> <hostId>linux-cl-2.intranet.gonicus.de</hostId> <productId>xpconfig</productId> </xml>";
    #$data = "<xml> <header>gosa_opsi_add_product_to_client</header> <source>GOSA</source> <target>00:01:6c:9d:b9:fa</target> <macaddress>00:11:25:4b:8c:e5</macaddress> <hostId>linux-cl-2.intranet.gonicus.de</hostId> <productId>hwaudit</productId> </xml>";
    #$data = "<xml> <header>gosa_opsi_add_product_to_client</header> <source>GOSA</source> <target>00:01:6c:9d:b9:fa</target> <macaddress>00:11:25:4b:8c:e5</macaddress> <hostId>linux-cl-2.intranet.gonicus.de</hostId> <productId>javavm</productId> </xml>";
    #$data = "<xml> <header>gosa_opsi_add_product_to_client</header> <source>GOSA</source> <target>00:01:6c:9d:b9:fa</target> <macaddress>00:11:25:4b:8c:e5</macaddress> <hostId>linux-cl-2.intranet.gonicus.de</hostId> <productId>firefox</productId> </xml>";
    #$data = "<xml> <header>gosa_opsi_add_product_to_client</header> <source>GOSA</source> <target>00:01:6c:9d:b9:fa</target> <macaddress>00:11:25:4b:8c:e5</macaddress> <hostId>linux-cl-2.intranet.gonicus.de</hostId> <productId>flashplayer</productId> </xml>";

    # Delete product from Opsi client
    #$data = "<xml> <header>gosa_opsi_del_product_from_client</header> <source>GOSA</source> <target>00:01:6c:9d:b9:fa</target> <hostId>linux-cl-1.intranet.gonicus.de</hostId> <macaddress>00:11:25:4b:8c:e5</macaddress> <productId>wipedisk</productId>  </xml>";

    #########################
    # Mailqueue communication
    
    # Writing data into a mailqueue
    #echo sabber | mail -s test horst@woauchimmer.de    

    # Querying the mailqueue at 
    #$data = "<xml> <header>gosa_mailqueue_query</header> <source>GOSA</source> <target>00:01:6c:9d:b9:fa</target> </xml>";
    #$data = "<xml> <header>gosa_mailqueue_query</header> <source>GOSA</source> <target>00:01:6c:9d:b9:fa</target> <q_tag>recipient</q_tag> <q_operator>eq</q_operator> <q_value>retta</q_value> </xml>";

    # Multiple xml tags msg_id are allowed
    #$data = "<xml> <header>gosa_mailqueue_hold</header> <source>GOSA</source> <target>00:01:6c:9d:b9:fa</target> <msg_id>99C8ABEF23</msg_id> </xml>";
    #$data = "<xml> <header>gosa_mailqueue_unhold</header> <source>GOSA</source> <target>00:01:6c:9d:b9:fa</target> <msg_id>5657EBEEF7</msg_id> </xml>";
    #$data = "<xml> <header>gosa_mailqueue_requeue</header> <source>GOSA</source> <target>00:01:6c:9d:b9:fa</target> <msg_id>11A09BEF04</msg_id> </xml>";
    #$data = "<xml> <header>gosa_mailqueue_del</header> <source>GOSA</source> <target>00:01:6c:9d:b9:fa</target> <msg_id>CEAFDBEF45</msg_id> </xml>";

    # Only one xml tag msg_id is allowed
    #$data = "<xml> <header>gosa_mailqueue_header</header> <source>GOSA</source> <target>00:01:6c:9d:b9:fa</target> <msg_id>EFD81BEF45</msg_id> </xml>";
     
    ########################
    # DAK Debian Archive Kit
    #$data = "<xml> <header>gosa_get_dak_keyring</header> <source>GOSA</source> <target>00:01:6c:9d:b9:fa</target> </xml>";
    #$data = "<xml> <header>gosa_import_dak_key</header> <source>GOSA</source> <target>00:01:6c:9d:b9:fa</target> </xml>";
    #$data = "<xml> <header>gosa_remove_dak_key</header> <source>GOSA</source> <target>00:01:6c:9d:b9:fa</target> </xml>";


    ##############################
    # NTP reload
    #$data = "<xml> <header>gosa_trigger_reload_ntp_config</header> <source>GOSA</source> <target>GOSA</target> <macaddress>00:11:25:4B:8C:E5</macaddress> </xml>"; 

    ##############################
    # SYSLOG reload
    #$data = "<xml> <header>gosa_trigger_reload_syslog_config</header> <source>GOSA</source> <target>GOSA</target> <macaddress>00:01:6c:9d:b9:fa</macaddress> </xml>"; 




    $sock->write($data);
    $answer = "nothing";
    $answer = $sock->read();

    echo "$count: $answer\n";
    $sock->close();	
  }else{
    echo "... FAILED!\n";
  }
}
?>
