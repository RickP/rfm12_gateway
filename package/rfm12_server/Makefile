#
# Copyright (C) 2010-2012 Jo-Philipp Wich <xm@subsignal.org>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=rfm12_server
PKG_RELEASE:=1
PKG_RELEASE:=1.0

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

DEPENDS:=@GPIO_SUPPORT +kmod-spi-gpio

include $(INCLUDE_DIR)/package.mk

define Package/rfm12_server
  SECTION:=net
  CATEGORY:=Network
  TITLE:=RFM12_server - a driver for the rfm12 network module
  MAINTAINER:=Rick Pannen <pannen@gmail.com>
endef

define Package/rfm12_server/description
endef


TARGET_CFLAGS += $(TLS_CFLAGS)
TARGET_CFLAGS += -I$(LINUX_DIR)/include
TARGET_LDFLAGS += -Wl,-rpath-link=$(STAGING_DIR)/usr/lib
MAKE_VARS += \
	FPIC="$(FPIC)" \
	TLS_CFLAGS="$(TLS_CFLAGS)" \
	TLS_LDFLAGS="$(TLS_LDFLAGS)" 

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/rfm12_server/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/rfm12.init $(1)/etc/init.d/rfm12
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/rfm12_server $(1)/sbin/rfm12_server
endef


$(eval $(call BuildPackage,rfm12_server))
