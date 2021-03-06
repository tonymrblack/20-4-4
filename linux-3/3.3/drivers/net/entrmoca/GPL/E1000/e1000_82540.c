/*******************************************************************************

  Intel PRO/1000 Linux driver
  Copyright(c) 1999 - 2008 Intel Corporation.

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Contact Information:
  Linux NICS <linux.nics@intel.com>
  e1000-devel Mailing List <e1000-devel@lists.sourceforge.net>
  Intel Corporation, 5200 N.E. Elam Young Parkway, Hillsboro, OR 97124-6497

*******************************************************************************/

/*******************************************************************************
Change Log:
[11/17/08]	Kevin Cernekee		e1000_82540.c
		Exclude the function static s32  e1000_set_phy_mode_82540(struct e1000_hw *hw);
		When define the ZIP_2 macro, and it will never be used for GMII+EN2210 card driver.

		Define several EN2210 related read/write functions.

		Register the EN2210 MMI read/write function to the adapter layer.

		Disable the phy chipset checking.

		Set LINK_MODE to use the external GMII interface, it enables the HW interface
		between 82545 and EN2210.

		In function static s32 e1000_setup_copper_link_82540(struct e1000_hw *hw)
		Exclude the e1000_set_phy_mode_82540() function, because the phy changed to EN2210, so it
		is illegel to set the other type phy mode here.

		static s32 e1000_set_phy_mode_82540(struct e1000_hw *hw)
		Exclude this function, because it is no meaning for this board.

		Create the follow function for talking with EN2210 via MMI bus.
		static s32 e1000_read_phy_reg_en2210(struct e1000_hw *hw, u32 offset, u16 *data)
		static s32 e1000_write_phy_reg_en2210(struct e1000_hw *hw, u32 offset, u16 data)
		static void e1000_raise_mdi_clk_en2210(struct e1000_hw *hw, u32 *ctrl)
		static void e1000_lower_mdi_clk_en2210(struct e1000_hw *hw, u32 *ctrl)
		static void e1000_shift_out_mdi_bits_en2210(struct e1000_hw *hw, u32 data,u16 count)
		static u16 e1000_shift_in_mdi_bits_en2210(struct e1000_hw *hw)

*******************************************************************************/

/* e1000_82540
 * e1000_82545
 * e1000_82546
 * e1000_82545_rev_3
 * e1000_82546_rev_3
 */

#include "e1000_api.h"
#include "entropic-config.h"

static s32  e1000_init_phy_params_82540(struct e1000_hw *hw);
static s32  e1000_init_nvm_params_82540(struct e1000_hw *hw);
static s32  e1000_init_mac_params_82540(struct e1000_hw *hw);
static s32  e1000_adjust_serdes_amplitude_82540(struct e1000_hw *hw);
static void e1000_clear_hw_cntrs_82540(struct e1000_hw *hw);
static s32  e1000_init_hw_82540(struct e1000_hw *hw);
static s32  e1000_reset_hw_82540(struct e1000_hw *hw);
#if 0 // Does not apply to Entropic products
static s32  e1000_set_phy_mode_82540(struct e1000_hw *hw);
#endif
static s32  e1000_set_vco_speed_82540(struct e1000_hw *hw);
static s32  e1000_setup_copper_link_82540(struct e1000_hw *hw);
static s32  e1000_setup_fiber_serdes_link_82540(struct e1000_hw *hw);
static void e1000_power_down_phy_copper_82540(struct e1000_hw *hw);
static s32 e1000_read_phy_reg_en2210(struct e1000_hw *hw, u32 offset, u16 *data);
static s32 e1000_write_phy_reg_en2210(struct e1000_hw *hw, u32 offset, u16 data);
static void e1000_raise_mdi_clk_en2210(struct e1000_hw *hw, u32 *ctrl); 
static void e1000_lower_mdi_clk_en2210(struct e1000_hw *hw, u32 *ctrl); 
static void e1000_shift_out_mdi_bits_en2210(struct e1000_hw *hw, u32 data,u16 count);

static u16 e1000_shift_in_mdi_bits_en2210(struct e1000_hw *hw);

/**
 * e1000_init_phy_params_82540 - Init PHY func ptrs.
 * @hw: pointer to the HW structure
 *
 * This is a function pointer entry point called by the api module.
 **/
static s32 e1000_init_phy_params_82540(struct e1000_hw *hw)
{
	struct e1000_phy_info *phy = &hw->phy;
	s32 ret_val = E1000_SUCCESS;

	phy->addr                      = 1;
	phy->autoneg_mask              = AUTONEG_ADVERTISE_SPEED_DEFAULT;
	phy->reset_delay_us            = 10000;
	phy->type                      = e1000_phy_m88;

	/* Function Pointers */
	phy->ops.check_polarity        = e1000_check_polarity_m88;
	phy->ops.commit                = e1000_phy_sw_reset_generic;
	phy->ops.force_speed_duplex    = e1000_phy_force_speed_duplex_m88;
	phy->ops.get_cable_length      = e1000_get_cable_length_m88;
	phy->ops.get_cfg_done          = e1000_get_cfg_done_generic;
	phy->ops.read_reg              = e1000_read_phy_reg_en2210;
	phy->ops.reset                 = e1000_phy_hw_reset_generic;
	phy->ops.write_reg             = e1000_write_phy_reg_en2210;
	phy->ops.get_info              = e1000_get_phy_info_m88;
	phy->ops.power_up              = e1000_power_up_phy_copper;
	phy->ops.power_down            = e1000_power_down_phy_copper_82540;
#if 0 // Does not apply to Entropic products
	ret_val = e1000_get_phy_id(hw);
	if (ret_val)
		goto out;

	/* Verify phy id */
	switch (hw->mac.type) {
	case e1000_82540:
	case e1000_82545:
	case e1000_82545_rev_3:
	case e1000_82546:
	case e1000_82546_rev_3:
		if (phy->id == M88E1011_I_PHY_ID)
			break;
		/* Fall Through */
	default:
		ret_val = -E1000_ERR_PHY;
		goto out;
		break;
	}

out:
#endif
	return ret_val;
}

/**
 * e1000_init_nvm_params_82540 - Init NVM func ptrs.
 * @hw: pointer to the HW structure
 *
 * This is a function pointer entry point called by the api module.
 **/
static s32 e1000_init_nvm_params_82540(struct e1000_hw *hw)
{
	struct e1000_nvm_info *nvm = &hw->nvm;
	u32 eecd = E1000_READ_REG(hw, E1000_EECD);

	DEBUGFUNC("e1000_init_nvm_params_82540");

	nvm->type               = e1000_nvm_eeprom_microwire;
	nvm->delay_usec         = 50;
	nvm->opcode_bits        = 3;
	switch (nvm->override) {
	case e1000_nvm_override_microwire_large:
		nvm->address_bits       = 8;
		nvm->word_size          = 256;
		break;
	case e1000_nvm_override_microwire_small:
		nvm->address_bits       = 6;
		nvm->word_size          = 64;
		break;
	default:
		nvm->address_bits       = eecd & E1000_EECD_SIZE ? 8 : 6;
		nvm->word_size          = eecd & E1000_EECD_SIZE ? 256 : 64;
		break;
	}

	/* Function Pointers */
	nvm->ops.acquire            = e1000_acquire_nvm_generic;
	nvm->ops.read               = e1000_read_nvm_microwire;
	nvm->ops.release            = e1000_release_nvm_generic;
	nvm->ops.update             = e1000_update_nvm_checksum_generic;
	nvm->ops.valid_led_default  = e1000_valid_led_default_generic;
	nvm->ops.validate           = e1000_validate_nvm_checksum_generic;
	nvm->ops.write              = e1000_write_nvm_microwire;

	return E1000_SUCCESS;
}

/**
 * e1000_init_mac_params_82540 - Init MAC func ptrs.
 * @hw: pointer to the HW structure
 *
 * This is a function pointer entry point called by the api module.
 **/
static s32 e1000_init_mac_params_82540(struct e1000_hw *hw)
{
	struct e1000_mac_info *mac = &hw->mac;
	s32 ret_val = E1000_SUCCESS;

	DEBUGFUNC("e1000_init_mac_params_82540");

	/* Set media type */
	switch (hw->device_id) {
	case E1000_DEV_ID_82545EM_FIBER:
	case E1000_DEV_ID_82545GM_FIBER:
	case E1000_DEV_ID_82546EB_FIBER:
	case E1000_DEV_ID_82546GB_FIBER:
		hw->phy.media_type = e1000_media_type_fiber;
		break;
	case E1000_DEV_ID_82545GM_SERDES:
	case E1000_DEV_ID_82546GB_SERDES:
		hw->phy.media_type = e1000_media_type_internal_serdes;
		break;
	default:
		hw->phy.media_type = e1000_media_type_copper;
		break;
	}

	/* Set mta register count */
	mac->mta_reg_count = 128;
	/* Set rar entry count */
	mac->rar_entry_count = E1000_RAR_ENTRIES;

	/* Function pointers */

	/* bus type/speed/width */
	mac->ops.get_bus_info = e1000_get_bus_info_pci_generic;
	/* reset */
	mac->ops.reset_hw = e1000_reset_hw_82540;
	/* hw initialization */
	mac->ops.init_hw = e1000_init_hw_82540;
	/* link setup */
	mac->ops.setup_link = e1000_setup_link_generic;
	/* physical interface setup */
	mac->ops.setup_physical_interface =
	        (hw->phy.media_type == e1000_media_type_copper)
	                ? e1000_setup_copper_link_82540
	                : e1000_setup_fiber_serdes_link_82540;
	/* check for link */
	switch (hw->phy.media_type) {
	case e1000_media_type_copper:
		mac->ops.check_for_link = e1000_check_for_copper_link_generic;
		break;
	case e1000_media_type_fiber:
		mac->ops.check_for_link = e1000_check_for_fiber_link_generic;
		break;
	case e1000_media_type_internal_serdes:
		mac->ops.check_for_link = e1000_check_for_serdes_link_generic;
		break;
	default:
		ret_val = -E1000_ERR_CONFIG;
		goto out;
		break;
	}
	/* link info */
	mac->ops.get_link_up_info =
	        (hw->phy.media_type == e1000_media_type_copper)
	                ? e1000_get_speed_and_duplex_copper_generic
	                : e1000_get_speed_and_duplex_fiber_serdes_generic;
	/* multicast address update */
	mac->ops.update_mc_addr_list = e1000_update_mc_addr_list_generic;
	/* writing VFTA */
	mac->ops.write_vfta = e1000_write_vfta_generic;
	/* clearing VFTA */
	mac->ops.clear_vfta = e1000_clear_vfta_generic;
	/* setting MTA */
	mac->ops.mta_set = e1000_mta_set_generic;
	/* setup LED */
	mac->ops.setup_led = e1000_setup_led_generic;
	/* cleanup LED */
	mac->ops.cleanup_led = e1000_cleanup_led_generic;
	/* turn on/off LED */
	mac->ops.led_on = e1000_led_on_generic;
	mac->ops.led_off = e1000_led_off_generic;
	/* clear hardware counters */
	mac->ops.clear_hw_cntrs = e1000_clear_hw_cntrs_82540;

out:
	return ret_val;
}

/**
 * e1000_init_function_pointers_82540 - Init func ptrs.
 * @hw: pointer to the HW structure
 *
 * The only function explicitly called by the api module to initialize
 * all function pointers and parameters.
 **/
void e1000_init_function_pointers_82540(struct e1000_hw *hw)
{
	DEBUGFUNC("e1000_init_function_pointers_82540");

	hw->mac.ops.init_params = e1000_init_mac_params_82540;
	hw->nvm.ops.init_params = e1000_init_nvm_params_82540;
	hw->phy.ops.init_params = e1000_init_phy_params_82540;
}

/**
 *  e1000_reset_hw_82540 - Reset hardware
 *  @hw: pointer to the HW structure
 *
 *  This resets the hardware into a known state.  This is a
 *  function pointer entry point called by the api module.
 **/
static s32 e1000_reset_hw_82540(struct e1000_hw *hw)
{
	u32 ctrl, icr, manc;
	s32 ret_val = E1000_SUCCESS;

	DEBUGFUNC("e1000_reset_hw_82540");

	DEBUGOUT("Masking off all interrupts\n");
	E1000_WRITE_REG(hw, E1000_IMC, 0xFFFFFFFF);

	E1000_WRITE_REG(hw, E1000_RCTL, 0);
	E1000_WRITE_REG(hw, E1000_TCTL, E1000_TCTL_PSP);
	E1000_WRITE_FLUSH(hw);

	/*
	 * Delay to allow any outstanding PCI transactions to complete
	 * before resetting the device.
	 */
	msec_delay(10);

	ctrl = E1000_READ_REG(hw, E1000_CTRL);

	DEBUGOUT("Issuing a global reset to 82540/82545/82546 MAC\n");
	switch (hw->mac.type) {
	case e1000_82545_rev_3:
	case e1000_82546_rev_3:
		E1000_WRITE_REG(hw, E1000_CTRL_DUP, ctrl | E1000_CTRL_RST);
		break;
	default:
		/*
		 * These controllers can't ack the 64-bit write when
		 * issuing the reset, so we use IO-mapping as a
		 * workaround to issue the reset.
		 */
		E1000_WRITE_REG_IO(hw, E1000_CTRL, ctrl | E1000_CTRL_RST);
		break;
	}

	/* Wait for EEPROM reload */
	msec_delay(5);

	/* Disable HW ARPs on ASF enabled adapters */
	manc = E1000_READ_REG(hw, E1000_MANC);
	manc &= ~E1000_MANC_ARP_EN;
	E1000_WRITE_REG(hw, E1000_MANC, manc);

	E1000_WRITE_REG(hw, E1000_IMC, 0xffffffff);
	icr = E1000_READ_REG(hw, E1000_ICR);

	return ret_val;
}

/**
 *  e1000_init_hw_82540 - Initialize hardware
 *  @hw: pointer to the HW structure
 *
 *  This inits the hardware readying it for operation.  This is a
 *  function pointer entry point called by the api module.
 **/
static s32 e1000_init_hw_82540(struct e1000_hw *hw)
{
	struct e1000_mac_info *mac = &hw->mac;
	u32 txdctl, ctrl_ext;
	s32 ret_val = E1000_SUCCESS;
	u16 i;

	DEBUGFUNC("e1000_init_hw_82540");

	/* Initialize identification LED */
	ret_val = e1000_id_led_init_generic(hw);
	if (ret_val) {
		DEBUGOUT("Error initializing identification LED\n");
		/* This is not fatal and we should not stop init due to this */
	}

	/* Disabling VLAN filtering */
	DEBUGOUT("Initializing the IEEE VLAN\n");
	if (mac->type < e1000_82545_rev_3)
		E1000_WRITE_REG(hw, E1000_VET, 0);

	mac->ops.clear_vfta(hw);

	/* Setup the receive address. */
	e1000_init_rx_addrs_generic(hw, mac->rar_entry_count);

	/* Zero out the Multicast HASH table */
	DEBUGOUT("Zeroing the MTA\n");
	for (i = 0; i < mac->mta_reg_count; i++) {
		E1000_WRITE_REG_ARRAY(hw, E1000_MTA, i, 0);
		/*
		 * Avoid back to back register writes by adding the register
		 * read (flush).  This is to protect against some strange
		 * bridge configurations that may issue Memory Write Block
		 * (MWB) to our register space.  The *_rev_3 hardware at
		 * least doesn't respond correctly to every other dword in an
		 * MWB to our register space.
		 */
		E1000_WRITE_FLUSH(hw);
	}

	if (mac->type < e1000_82545_rev_3)
		e1000_pcix_mmrbc_workaround_generic(hw);

	/* Setup link and flow control */
	ret_val = mac->ops.setup_link(hw);

	txdctl = E1000_READ_REG(hw, E1000_TXDCTL(0));
	txdctl = (txdctl & ~E1000_TXDCTL_WTHRESH) |
	         E1000_TXDCTL_FULL_TX_DESC_WB;
	E1000_WRITE_REG(hw, E1000_TXDCTL(0), txdctl);


#if !ECFG_CHIP_ZIP1
    /* set LINK_MODE to use external GMII interface */
    ctrl_ext = E1000_READ_REG(hw, E1000_CTRL_EXT);
    ctrl_ext &= ~E1000_CTRL_EXT_LINK_MODE_MASK;
    ctrl_ext |= E1000_CTRL_EXT_BITS;
    E1000_WRITE_REG(hw, E1000_CTRL_EXT, ctrl_ext);
    E1000_WRITE_FLUSH(hw);
#endif /* (!ECFG_CHIP_ZIP1) */

	/*
	 * Clear all of the statistics registers (clear on read).  It is
	 * important that we do this after we have tried to establish link
	 * because the symbol error count will increment wildly if there
	 * is no link.
	 */
	e1000_clear_hw_cntrs_82540(hw);

	if ((hw->device_id == E1000_DEV_ID_82546GB_QUAD_COPPER) ||
	    (hw->device_id == E1000_DEV_ID_82546GB_QUAD_COPPER_KSP3)) {
		ctrl_ext = E1000_READ_REG(hw, E1000_CTRL_EXT);
		/*
		 * Relaxed ordering must be disabled to avoid a parity
		 * error crash in a PCI slot.
		 */
		ctrl_ext |= E1000_CTRL_EXT_RO_DIS;
		E1000_WRITE_REG(hw, E1000_CTRL_EXT, ctrl_ext);
	}

	return ret_val;
}

/**
 *  e1000_setup_copper_link_82540 - Configure copper link settings
 *  @hw: pointer to the HW structure
 *
 *  Calls the appropriate function to configure the link for auto-neg or forced
 *  speed and duplex.  Then we check for link, once link is established calls
 *  to configure collision distance and flow control are called.  If link is
 *  not established, we return -E1000_ERR_PHY (-2).  This is a function
 *  pointer entry point called by the api module.
 **/
static s32 e1000_setup_copper_link_82540(struct e1000_hw *hw)
{
	u32 ctrl;
	s32 ret_val = E1000_SUCCESS;
#if 0 // Does not apply to Entropic products
	u16 data;
#endif
	DEBUGFUNC("e1000_setup_copper_link_82540");

	ctrl = E1000_READ_REG(hw, E1000_CTRL);
	ctrl |= E1000_CTRL_SLU;
	ctrl &= ~(E1000_CTRL_FRCSPD | E1000_CTRL_FRCDPX);
	E1000_WRITE_REG(hw, E1000_CTRL, ctrl);
#if 0 // Does not apply to Entropic products
	ret_val = e1000_set_phy_mode_82540(hw);
	if (ret_val)
		goto out;

	if (hw->mac.type == e1000_82545_rev_3 ||
	    hw->mac.type == e1000_82546_rev_3) {
		ret_val = hw->phy.ops.read_reg(hw, M88E1000_PHY_SPEC_CTRL, &data);
		if (ret_val)
			goto out;
		data |= 0x00000008;
		ret_val = hw->phy.ops.write_reg(hw, M88E1000_PHY_SPEC_CTRL, data);
		if (ret_val)
			goto out;
	}

	ret_val = e1000_copper_link_setup_m88(hw);
	if (ret_val)
		goto out;
#endif
	ret_val = e1000_setup_copper_link_generic(hw);

#if 0 // Does not apply to Entropic products
out:
#endif
	return ret_val;
}

/**
 *  e1000_setup_fiber_serdes_link_82540 - Setup link for fiber/serdes
 *  @hw: pointer to the HW structure
 *
 *  Set the output amplitude to the value in the EEPROM and adjust the VCO
 *  speed to improve Bit Error Rate (BER) performance.  Configures collision
 *  distance and flow control for fiber and serdes links.  Upon successful
 *  setup, poll for link.  This is a function pointer entry point called by
 *  the api module.
 **/
static s32 e1000_setup_fiber_serdes_link_82540(struct e1000_hw *hw)
{
	struct e1000_mac_info *mac = &hw->mac;
	s32 ret_val = E1000_SUCCESS;

	DEBUGFUNC("e1000_setup_fiber_serdes_link_82540");

	switch (mac->type) {
	case e1000_82545_rev_3:
	case e1000_82546_rev_3:
		if (hw->phy.media_type == e1000_media_type_internal_serdes) {
			/*
			 * If we're on serdes media, adjust the output
			 * amplitude to value set in the EEPROM.
			 */
			ret_val = e1000_adjust_serdes_amplitude_82540(hw);
			if (ret_val)
				goto out;
		}
		/* Adjust VCO speed to improve BER performance */
		ret_val = e1000_set_vco_speed_82540(hw);
		if (ret_val)
			goto out;
	default:
		break;
	}

	ret_val = e1000_setup_fiber_serdes_link_generic(hw);

out:
	return ret_val;
}

/**
 *  e1000_adjust_serdes_amplitude_82540 - Adjust amplitude based on EEPROM
 *  @hw: pointer to the HW structure
 *
 *  Adjust the SERDES output amplitude based on the EEPROM settings.
 **/
static s32 e1000_adjust_serdes_amplitude_82540(struct e1000_hw *hw)
{
	s32 ret_val = E1000_SUCCESS;
	u16 nvm_data;

	DEBUGFUNC("e1000_adjust_serdes_amplitude_82540");

	ret_val = hw->nvm.ops.read(hw, NVM_SERDES_AMPLITUDE, 1, &nvm_data);
	if (ret_val)
		goto out;

	if (nvm_data != NVM_RESERVED_WORD) {
		/* Adjust serdes output amplitude only. */
		nvm_data &= NVM_SERDES_AMPLITUDE_MASK;
		ret_val = hw->phy.ops.write_reg(hw,
		                             M88E1000_PHY_EXT_CTRL,
		                             nvm_data);
		if (ret_val)
			goto out;
	}

out:
	return ret_val;
}

/**
 *  e1000_set_vco_speed_82540 - Set VCO speed for better performance
 *  @hw: pointer to the HW structure
 *
 *  Set the VCO speed to improve Bit Error Rate (BER) performance.
 **/
static s32 e1000_set_vco_speed_82540(struct e1000_hw *hw)
{
	s32  ret_val = E1000_SUCCESS;
	u16 default_page = 0;
	u16 phy_data;

	DEBUGFUNC("e1000_set_vco_speed_82540");

	/* Set PHY register 30, page 5, bit 8 to 0 */

	ret_val = hw->phy.ops.read_reg(hw,
	                            M88E1000_PHY_PAGE_SELECT,
	                            &default_page);
	if (ret_val)
		goto out;

	ret_val = hw->phy.ops.write_reg(hw, M88E1000_PHY_PAGE_SELECT, 0x0005);
	if (ret_val)
		goto out;

	ret_val = hw->phy.ops.read_reg(hw, M88E1000_PHY_GEN_CONTROL, &phy_data);
	if (ret_val)
		goto out;

	phy_data &= ~M88E1000_PHY_VCO_REG_BIT8;
	ret_val = hw->phy.ops.write_reg(hw, M88E1000_PHY_GEN_CONTROL, phy_data);
	if (ret_val)
		goto out;

	/* Set PHY register 30, page 4, bit 11 to 1 */

	ret_val = hw->phy.ops.write_reg(hw, M88E1000_PHY_PAGE_SELECT, 0x0004);
	if (ret_val)
		goto out;

	ret_val = hw->phy.ops.read_reg(hw, M88E1000_PHY_GEN_CONTROL, &phy_data);
	if (ret_val)
		goto out;

	phy_data |= M88E1000_PHY_VCO_REG_BIT11;
	ret_val = hw->phy.ops.write_reg(hw, M88E1000_PHY_GEN_CONTROL, phy_data);
	if (ret_val)
		goto out;

	ret_val = hw->phy.ops.write_reg(hw, M88E1000_PHY_PAGE_SELECT,
	                              default_page);

out:
	return ret_val;
}

#if 0 // Does not apply to Entropic products

/**
 *  e1000_set_phy_mode_82540 - Set PHY to class A mode
 *  @hw: pointer to the HW structure
 *
 *  Sets the PHY to class A mode and assumes the following operations will
 *  follow to enable the new class mode:
 *    1.  Do a PHY soft reset.
 *    2.  Restart auto-negotiation or force link.
 **/
static s32 e1000_set_phy_mode_82540(struct e1000_hw *hw)
{
	struct e1000_phy_info *phy = &hw->phy;
	s32 ret_val = E1000_SUCCESS;
	u16 nvm_data;

	DEBUGFUNC("e1000_set_phy_mode_82540");

	if (hw->mac.type != e1000_82545_rev_3)
		goto out;

	ret_val = hw->nvm.ops.read(hw, NVM_PHY_CLASS_WORD, 1, &nvm_data);
	if (ret_val) {
		ret_val = -E1000_ERR_PHY;
		goto out;
	}

	if ((nvm_data != NVM_RESERVED_WORD) && (nvm_data & NVM_PHY_CLASS_A)) {
		ret_val = hw->phy.ops.write_reg(hw, M88E1000_PHY_PAGE_SELECT,
		                              0x000B);
		if (ret_val) {
			ret_val = -E1000_ERR_PHY;
			goto out;
		}
		ret_val = hw->phy.ops.write_reg(hw,
		                              M88E1000_PHY_GEN_CONTROL,
		                              0x8104);
		if (ret_val) {
			ret_val = -E1000_ERR_PHY;
			goto out;
		}

		phy->reset_disable = FALSE;
	}

out:
	return ret_val;
}
#endif
/**
 * e1000_power_down_phy_copper_82540 - Remove link in case of PHY power down
 * @hw: pointer to the HW structure
 *
 * In the case of a PHY power down to save power, or to turn off link during a
 * driver unload, or wake on lan is not enabled, remove the link.
 **/
static void e1000_power_down_phy_copper_82540(struct e1000_hw *hw)
{
	/* If the management interface is not enabled, then power down */
	if (!(E1000_READ_REG(hw, E1000_MANC) & E1000_MANC_SMBUS_EN))
		e1000_power_down_phy_copper(hw);

	return;
}

/**
 *  e1000_clear_hw_cntrs_82540 - Clear device specific hardware counters
 *  @hw: pointer to the HW structure
 *
 *  Clears the hardware counters by reading the counter registers.
 **/
static void e1000_clear_hw_cntrs_82540(struct e1000_hw *hw)
{
	volatile u32 temp;

	DEBUGFUNC("e1000_clear_hw_cntrs_82540");

	e1000_clear_hw_cntrs_base_generic(hw);

	temp = E1000_READ_REG(hw, E1000_PRC64);
	temp = E1000_READ_REG(hw, E1000_PRC127);
	temp = E1000_READ_REG(hw, E1000_PRC255);
	temp = E1000_READ_REG(hw, E1000_PRC511);
	temp = E1000_READ_REG(hw, E1000_PRC1023);
	temp = E1000_READ_REG(hw, E1000_PRC1522);
	temp = E1000_READ_REG(hw, E1000_PTC64);
	temp = E1000_READ_REG(hw, E1000_PTC127);
	temp = E1000_READ_REG(hw, E1000_PTC255);
	temp = E1000_READ_REG(hw, E1000_PTC511);
	temp = E1000_READ_REG(hw, E1000_PTC1023);
	temp = E1000_READ_REG(hw, E1000_PTC1522);

	temp = E1000_READ_REG(hw, E1000_ALGNERRC);
	temp = E1000_READ_REG(hw, E1000_RXERRC);
	temp = E1000_READ_REG(hw, E1000_TNCRS);
	temp = E1000_READ_REG(hw, E1000_CEXTERR);
	temp = E1000_READ_REG(hw, E1000_TSCTC);
	temp = E1000_READ_REG(hw, E1000_TSCTFC);

	temp = E1000_READ_REG(hw, E1000_MGTPRC);
	temp = E1000_READ_REG(hw, E1000_MGTPDC);
	temp = E1000_READ_REG(hw, E1000_MGTPTC);
}
#define PHY_PREAMBLE      0xFFFFFFFF
#define PHY_PREAMBLE_SIZE 32
#define PHY_SOF           0x1
#define PHY_OP_READ       0x2
#define PHY_OP_WRITE      0x1
#define PHY_TURNAROUND    0x2


/*
 *  e1000_read_phy_reg_en2210 - Read PHY register
 *  @hw: pointer to the HW structure
 *  @offset: register offset to be read
 *  @data: pointer to the read data
 *
 *  Reads the PHY at offset and stores the information read to data.
 **/
static s32 e1000_read_phy_reg_en2210(struct e1000_hw *hw, u32 offset, u16 *data)
{
	u32 mdic;
	s32 ret_val = E1000_SUCCESS;
    

	DEBUGFUNC("e1000_read_phy_reg_82543");

	if (offset > MAX_PHY_REG_ADDRESS) {
		DEBUGOUT1("PHY Address %d is out of range\n", offset);
		ret_val = -E1000_ERR_PARAM;
		goto out;
	}
#if !ECFG_CHIP_ZIP1
    spin_lock_irq(&hw->phy.mdio_lock);
    //spin_lock_irqsave(&hw->phy.mdio_lock, hw->phy.lock_irq_flags);

	/*
	 * We must first send a preamble through the MDIO pin to signal the
	 * beginning of an MII instruction.  This is done by sending 32
	 * consecutive "1" bits.
	 */
	e1000_shift_out_mdi_bits_en2210(hw, PHY_PREAMBLE, PHY_PREAMBLE_SIZE);

	/*
	 * Now combine the next few fields that are required for a read
	 * operation.  We use this method instead of calling the
	 * e1000_shift_out_mdi_bits routine five different times.  The format
	 * of an MII read instruction consists of a shift out of 14 bits and
	 * is defined as follows:
	 * 	<Preamble><SOF><Op Code><Phy Addr><Offset>
	 * followed by a shift in of 18 bits.  This first two bits shifted in
	 * are TurnAround bits used to avoid contention on the MDIO pin when a
	 * READ operation is performed.  These two bits are thrown away
	 * followed by a shift in of 16 bits which contains the desired data.
	 */
	mdic = (offset | (hw->phy.addr << 5) |
		(PHY_OP_READ << 10) | (PHY_SOF << 12));

	e1000_shift_out_mdi_bits_en2210(hw, mdic, 14);

	/*
	 * Now that we've shifted out the read command to the MII, we need to
	 * "shift in" the 16-bit value (18 total bits) of the requested PHY
	 * register address.
	 */
	*data = e1000_shift_in_mdi_bits_en2210(hw);
    spin_unlock_irq(&hw->phy.mdio_lock);
    //spin_unlock_irqrestore(&hw->phy.mdio_lock, hw->phy.lock_irq_flags);

#endif /* (!ECFG_CHIP_ZIP1 */

out:
	return ret_val;
}

/**
 *  e1000_write_phy_reg_en2210 - Write PHY register
 *  @hw: pointer to the HW structure
 *  @offset: register offset to be written
 *  @data: pointer to the data to be written at offset
 *
 *  Writes data to the PHY at offset.
 **/

static s32 e1000_write_phy_reg_en2210(struct e1000_hw *hw, u32 offset, u16 data)
{
	u32 mdic;
	s32 ret_val = E1000_SUCCESS;
	
	DEBUGFUNC("e1000_write_phy_reg_82543");

	if (offset > MAX_PHY_REG_ADDRESS) {
		DEBUGOUT1("PHY Address %d is out of range\n", offset);
		ret_val = -E1000_ERR_PARAM;
		goto out;
	}
#if !ECFG_CHIP_ZIP1
    spin_lock_irq(&hw->phy.mdio_lock);
    //spin_lock_irqsave(&hw->phy.mdio_lock, hw->phy.lock_irq_flags);

	/*
	 * We'll need to use the SW defined pins to shift the write command
	 * out to the PHY. We first send a preamble to the PHY to signal the
	 * beginning of the MII instruction.  This is done by sending 32
	 * consecutive "1" bits.
	 */
	e1000_shift_out_mdi_bits_en2210(hw, PHY_PREAMBLE, PHY_PREAMBLE_SIZE);

	/*
	 * Now combine the remaining required fields that will indicate a
	 * write operation. We use this method instead of calling the
	 * e1000_shift_out_mdi_bits routine for each field in the command. The
	 * format of a MII write instruction is as follows:
	 * <Preamble><SOF><Op Code><Phy Addr><Reg Addr><Turnaround><Data>.
	 */
	mdic = ((PHY_TURNAROUND) | (offset << 2) | (hw->phy.addr << 7) |
	        (PHY_OP_WRITE << 12) | (PHY_SOF << 14));
	mdic <<= 16;
	mdic |= (u32) data;

	e1000_shift_out_mdi_bits_en2210(hw, mdic, 32);
    spin_unlock_irq(&hw->phy.mdio_lock);
    //spin_unlock_irqrestore(&hw->phy.mdio_lock, hw->phy.lock_irq_flags);

#endif /* (!ECFG_CHIP_ZIP1) */

out:
	
	return ret_val;
}

/**
 *  e1000_raise_mdi_clk_en2210 - Raise Management Data Input clock
 *  @hw: pointer to the HW structure
 *  @ctrl: pointer to the control register
 *
 *  Raise the management data input clock by setting the MDC bit in the control
 *  register.
 **/


static void e1000_raise_mdi_clk_en2210(struct e1000_hw *hw, u32 *ctrl)
{
	/*
	 * Raise the clock input to the Management Data Clock (by setting the
	 * MDC bit), and then delay a sufficient amount of time.
	 */
	E1000_WRITE_REG(hw, E1000_CTRL, (*ctrl | E1000_CTRL_MDC));
#if !ECFG_CHIP_ZIP1
    if(! (hw->phy.z2_flags & Z2_FLAG_FAST_MDIO))
        E1000_WRITE_FLUSH(hw);
    ndelay(20);
#else /* (!ECFG_CHIP_ZIP1) */
    E1000_WRITE_FLUSH(hw);
    udelay(10);
#endif /* (!ECFG_CHIP_ZIP1) */
}

/**
 *  e1000_lower_mdi_clk_en2210 - Lower Management Data Input clock
 *  @hw: pointer to the HW structure
 *  @ctrl: pointer to the control register
 *
 *  Lower the management data input clock by clearing the MDC bit in the
 *  control register.
 **/


static void e1000_lower_mdi_clk_en2210(struct e1000_hw *hw, u32 *ctrl)
{
	/*
	 * Lower the clock input to the Management Data Clock (by clearing the
	 * MDC bit), and then delay a sufficient amount of time.
	 */	
	 E1000_WRITE_REG(hw, E1000_CTRL, (*ctrl & ~E1000_CTRL_MDC));

#if !ECFG_CHIP_ZIP1
    if(! (hw->phy.z2_flags & Z2_FLAG_FAST_MDIO))
        E1000_WRITE_FLUSH(hw);
    ndelay(20);
#else /* (!ECFG_CHIP_ZIP1) */
    E1000_WRITE_FLUSH(hw);
    udelay(10);
#endif /* (!ECFG_CHIP_ZIP1) */
	 
}

/**
 *  e1000_shift_out_mdi_bits_en2210 - Shift data bits our to the PHY
 *  @hw: pointer to the HW structure
 *  @data: data to send to the PHY
 *  @count: number of bits to shift out
 *
 *  We need to shift 'count' bits out to the PHY.  So, the value in the
 *  "data" parameter will be shifted out to the PHY one bit at a time.
 *  In order to do this, "data" must be broken down into bits.
 **/
static void e1000_shift_out_mdi_bits_en2210(struct e1000_hw *hw, u32 data,
                                           u16 count)
{
	u32 ctrl, mask;

	/*
	 * We need to shift "count" number of bits out to the PHY.  So, the
	 * value in the "data" parameter will be shifted out to the PHY one
	 * bit at a time.  In order to do this, "data" must be broken down
	 * into bits.
	 */
	mask = 0x01;
	mask <<= (count -1);

	ctrl = E1000_READ_REG(hw, E1000_CTRL);

	/* Set MDIO_DIR and MDC_DIR direction bits to be used as output pins. */
	ctrl |= (E1000_CTRL_MDIO_DIR | E1000_CTRL_MDC_DIR);

	while (mask) {
		/*
		 * A "1" is shifted out to the PHY by setting the MDIO bit to
		 * "1" and then raising and lowering the Management Data Clock.
		 * A "0" is shifted out to the PHY by setting the MDIO bit to
		 * "0" and then raising and lowering the clock.
		 */
		if (data & mask) ctrl |= E1000_CTRL_MDIO;
		else ctrl &= ~E1000_CTRL_MDIO;

        e1000_lower_mdi_clk_en2210(hw, &ctrl);
        e1000_raise_mdi_clk_en2210(hw, &ctrl);

		mask >>= 1;
	}
}

/**
 *  e1000_shift_in_mdi_bits_en2210 - Shift data bits in from the PHY
 *  @hw: pointer to the HW structure
 *
 *  In order to read a register from the PHY, we need to shift 18 bits
 *  in from the PHY.  Bits are "shifted in" by raising the clock input to
 *  the PHY (setting the MDC bit), and then reading the value of the data out
 *  MDIO bit.
 **/

static u16 e1000_shift_in_mdi_bits_en2210(struct e1000_hw *hw)
{
	u32 ctrl;
	u16 data = 0;
	u8 i;

	/*
	 * In order to read a register from the PHY, we need to shift in a
	 * total of 18 bits from the PHY.  The first two bit (turnaround)
	 * times are used to avoid contention on the MDIO pin when a read
	 * operation is performed.  These two bits are ignored by us and
	 * thrown away.  Bits are "shifted in" by raising the input to the
	 * Management Data Clock (setting the MDC bit) and then reading the
	 * value of the MDIO bit.
	 */
	ctrl = E1000_READ_REG(hw, E1000_CTRL);

	/*
	 * Clear MDIO_DIR (SWDPIO1) to indicate this bit is to be used as
	 * input.
	 */
	ctrl &= ~E1000_CTRL_MDIO_DIR;
	ctrl &= ~E1000_CTRL_MDIO;

	E1000_WRITE_REG(hw, E1000_CTRL, ctrl);
	E1000_WRITE_FLUSH(hw);

	/*
	 * Raise and lower the clock before reading in the data.  This accounts
	 * for the turnaround bits.  The first clock occurred when we clocked
	 * out the last bit of the Register Address.
	 */
	e1000_lower_mdi_clk_en2210(hw, &ctrl);
	e1000_raise_mdi_clk_en2210(hw, &ctrl);

	for (data = 0, i = 0; i < 17; i++) {
        data = data << 1;
        e1000_lower_mdi_clk_en2210(hw, &ctrl);
        ctrl = E1000_READ_REG(hw, E1000_CTRL);
        /* Check to see if we shifted in a "1". */
        if(ctrl & E1000_CTRL_MDIO) data |= 1;
        e1000_raise_mdi_clk_en2210(hw, &ctrl);
	}

	e1000_lower_mdi_clk_en2210(hw, &ctrl);
	e1000_raise_mdi_clk_en2210(hw, &ctrl);
	return data;
}
