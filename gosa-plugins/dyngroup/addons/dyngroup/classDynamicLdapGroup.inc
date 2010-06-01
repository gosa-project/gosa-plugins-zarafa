<?php

/*
 * This code is part of GOsa (http://www.gosa-project.org)
 * Copyright (C) 2003-2008 GONICUS GmbH
 * Copyright (C) 2010 Thomas CHEMINEAU
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/**
 * This class will allow user to parameter dynamic group.
 * @author Thomas Chemineau - thomas.chemineau<at>gmail.com
 * @version 0.01
 */
class DynamicLdapGroup extends plugin
{

    /**
     * The attribute that will use GOsa to store LDAP URI.
     * @var array
     */
    public $attributes = array('labeledURI');

    /**
     * The objectClass that will use GOsa to identify a group as dynamic.
     * @var array
     */
    public $objectclasses = array('labeledURIObject');

    /**
     * Default value for the corresponding attribute found in the $this->attributes
     * array of this plugin.
     * @var string
     */
    public $labeledURI = 'ldap:///dc=example,dc=com?memberUid?sub?(objectClass=posixGroup)';

    /**
     * Store values of memberUrl.
     * @var Array
     */
    private $_memberUrls = Array();

    public $orig_dn ="";

    /**
     * Create this object.
     * @param Array $config GOsa config.
     * @param string $dn Current DN.
     */
    public function __construct ($config, $dn)
    {
        parent::__construct($config, $dn);

        // Preset the labeled URI with a valid value.
        if(!$this->is_account){
            $attr_label = $this->attributes[0];
            $this->$attr_label = str_replace('dc=example,dc=com', LDAP::fix($this->dn), $this->$attr_label);
        }

        $this->orig_dn = $this->dn;
    }

    public function check ()
    {
        return Array();
    }


    /**
     * Execute this plugin.
     * @return string HTML to print.
     */
    public function execute ()
    {
        //
        // Are we trying to modify state of this group ? If so,
        // we can edit the current object.
        //
        if (isset($_POST['modify_state']))
        {
            $this->is_account = !$this->is_account;
        }

        //
        // Display a message if this feature is disabled.
        //
        if (!$this->is_account)
        {
            return $this->show_disable_header(msgPool::addFeaturesButton(_("Dynamic Group")), msgPool::featuresDisabled(_("Dynamic Group")));
        }
        $display = $this->show_disable_header(msgPool::removeFeaturesButton(_("Dynamic Group")), msgPool::featuresEnabled(_("Dynamic Group")));

        //
        // Now, we search for current attributes, and display them.
        //
        $attr_label = $this->attributes[0];
        $attr_value = $this->$attr_label;

        // Display values.
        //
        $smarty = get_smarty();
        $smarty->assign('memberURLAttributeLabel', $attr_label);
        $smarty->assign('memberURLAttributeValue', $attr_value);
        $display .= $smarty->fetch(get_template_path('dyngroup.tpl', TRUE, dirname(__FILE__)));
        return $display;
    }



    /**
     * This plugin does nothing when this method is invoked.
     */
    public function remove_from_parent ()
    {
        parent::remove_from_parent();
        $ldap = $this->config->get_ldap_link();
        $ldap->cd($this->dn);
        $ldap->modify($this->attrs);
        if(!$ldap->success()){
            msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_DEL, get_class()));
        }
        return;
    }


    /**
     * This function is called when tab is undisplayed. For example, the current user
     * wants to change other settings of this group, but not save it to the LDAP
     * directory directly.
     */
    public function save_object ()
    {
        parent::save_object();
    }


    /**
     * That will add additionnal information into the current LDAP entry.
     * If this plugin is disable, then it will remove any data that references
     * this plugin into the LDAP directory.
     * @return boolean
     */
    public function save ()
    {
        parent::save();
        $this->cleanup();
        $ldap = $this->config->get_ldap_link();
        $ldap->cd($this->dn);
        $ldap->modify($this->attrs);
        if(!$ldap->success()){
            msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_DEL, get_class()));
        }

        if (strcasecmp($this->orig_dn, $this->dn) != 0)
        {
            $this->renameDNsInDynGroupsValues($this->orig_dn, $this->dn);
        }
    }


    /**
     * Return attributes values of an LDAP entry.
     * @param String $dn DN of the LDAP entry.
     * @param Array $attributes Attributes to look for.
     * @return Array An associative array of requested values.
     */
    public function getAttributesValues ($dn, $attributes = Array('dn'))
    {
        $ldap = $this->config->get_ldap_link();
        $ldap->cat($dn, $attributes);
        if ($attrs = $ldap->fetch())
        {
            $data = Array();
            foreach ($attributes as $attribute)
            {
                if (array_key_exists($attribute, $attrs) !== false)
                {
                    $data[$attribute] = $attrs[$attribute];
                    unset($data[$attribute]['count']);
                }
            }
            if (sizeof($data) > 0)
            {
                return $data;
            }
        }
        return false;
    }


    /**
     * Modify search base for all URL of all dynamic groups objects into the LDAP
     * directory.
     */
    public function renameDNsInDynGroupsValues ($old_dn, $new_dn)
    {
        $ldap = $this->config->get_ldap_link();
        $ldap->cd($this->config->current['BASE']);
        //
        // Build the LDAP search filter. We take only LDAP entries which have all
        // objectClasses and attributes defined by this plugin.
        //
        $filter = '';
        foreach ($this->objectclasses as $objectclass)
        {
            $filter .= '(objectClass=' . $objectclass . ')';
                    }
                    foreach ($this->attributes as $attribute)
                    {
                    $filter .= '(' . $attribute . '=*)';
                    }
            $filter = '(&' . $filter . ')';
            //
            // The search should return some LDAP entries. If so, performed modifications
            // on values (delete the values, and add it again with correct search DN).
            //
            $ldap->search($filter, Array('dn'));
            if ($attrs = $ldap->fetch())
            {
                foreach ($attrs as $dn)
                {
                    $values = $this->getAttributesValues($dn, $this->attributes);
                    if ($values === false || !is_array($values))
                    {
                        continue;
                    }
                    foreach ($values as $attribute => $value)
                    {
                        for($i=0; $i<sizeof($value); $i++)
                        {
                            $values[$attribute][$i] = str_replace($old_dn, $new_dn, $values[$attribute][$i]);
                        }
                    }
                    $ldap->cd($dn);
                    $ldap->modify($values);
                }
            }
    }


    /**
     * Static method to set ACL for this plugin.
     */
    public static function plInfo()
    {
        return Array(
                "plShortName"   => _("dyngroup"),
                "plDescription" => _("Dynamic group setting"),
                "plSelfModify"  => TRUE,
                "plDepends"     => Array(),
                "plPriority"    => 1,
                "plSection"     => Array("addon"),
                "plCategory"    => Array("groups", "department", "ogroups"),
                "plProvidedAcls" => array(
                    'labeledURI' =>  _('labeledURI'),
                    )
                );
    }
}

?>
