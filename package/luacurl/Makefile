#
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile 6994 2007-04-18 00:46:40Z nico $

include $(TOPDIR)/rules.mk

PKG_NAME:=luacurl
PKG_VERSION:=1.2.1
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/luacurl
  SUBMENU:=Lua
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=luacurl library for the LUA programming language
  URL:=http://www.luaforge.net/projects/luacurl
  DEPENDS:=+lua +libcurl +libz +libssl +libcrypto
endef

define Package/luacurl/description
	This package contains the luacurl library, a binding for the libcurl easy interface for lua.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

TARGET_LDFLAGS+=-shared
TARGET_LDFLAGS+=-Wl,-rpath-link=$(STAGING_DIR)/usr/lib
define Build/Compile	
	$(MAKE) -C $(PKG_BUILD_DIR) $(TARGET_CONFIGURE_OPTS) LDFLAGS="$(TARGET_LDFLAGS)" CFLAGS="$(TARGET_CFLAGS) -I$(LINUX_DIR)/include -I$(STAGING_DIR)/usr/include" 
endef

define Package/luacurl/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/
	$(CP) $(PKG_BUILD_DIR)/luacurl.so \
	$(1)/usr/lib/lua/luacurl.so
endef

$(eval $(call BuildPackage,luacurl))
