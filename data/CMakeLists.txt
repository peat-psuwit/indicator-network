
###########################
# Indicator-network service
###########################

set(
  INDICATOR_DIR
  "${CMAKE_INSTALL_DATADIR}/unity/indicators"
  CACHE FILEPATH "Indicator directory"
)

install(
  FILES "com.canonical.indicator.network"
  DESTINATION "${INDICATOR_DIR}"
)

###########################
# Indicator-network upstart job
###########################

set(
  NETWORK_SERVICE
  "${CMAKE_INSTALL_FULL_LIBEXECDIR}/indicator-network/indicator-network-service"
)

configure_file (
  indicator-network.conf.in
  indicator-network.conf
  @ONLY
)

install (
  FILES "${CMAKE_CURRENT_BINARY_DIR}/indicator-network.conf"
  DESTINATION "${CMAKE_INSTALL_DATADIR}/upstart/sessions/"
)

###########################
# indicator-network-secret-agent upstart job
###########################

set(
  AGENT_SERVICE
  "${CMAKE_INSTALL_FULL_LIBEXECDIR}/indicator-network/indicator-network-secret-agent"
)

configure_file (
  indicator-network-secret-agent.conf.in
  indicator-network-secret-agent.conf
  @ONLY
)

install (
  FILES "${CMAKE_CURRENT_BINARY_DIR}/indicator-network-secret-agent.conf"
  DESTINATION "${CMAKE_INSTALL_DATADIR}/upstart/sessions/"
)

###########################
# Indicator-network systemd jobs
###########################

# Uncomment when we drop Vivid
# pkg_get_variable(SYSTEMD_USER_DIR systemd systemduserunitdir)
set (SYSTEMD_USER_DIR "/usr/lib/systemd/user")
message (STATUS "${SYSTEMD_USER_DIR} is the systemd user unit file install dir")

configure_file (
  indicator-network-secret-agent.service.in
  indicator-network-secret-agent.service
  @ONLY
)

configure_file (
  indicator-network.service.in
  indicator-network.service
  @ONLY
)

install (
  FILES "${CMAKE_CURRENT_BINARY_DIR}/indicator-network-secret-agent.service"
  DESTINATION ${SYSTEMD_USER_DIR}
)

install (
  FILES "${CMAKE_CURRENT_BINARY_DIR}/indicator-network.service"
  DESTINATION ${SYSTEMD_USER_DIR}
)

###########################
# Indicator-network systemd Upstart overrides
###########################

set (SYSTEMD_UPSTART_DIR "${CMAKE_INSTALL_FULL_DATADIR}/upstart/systemd-session/upstart")
install (
  FILES indicator-network.override indicator-network-secret-agent.override
  DESTINATION ${SYSTEMD_UPSTART_DIR}
)

###########################
# GSettings
###########################

add_schema ("com.canonical.indicator.network.gschema.xml")

###########################
# XDG Autostart File
###########################

# where to install
set (XDG_AUTOSTART_DIR "/etc/xdg/autostart")
message (STATUS "${XDG_AUTOSTART_DIR} is the DBus Service File install dir")

set (XDG_AUTOSTART_NAME "${CMAKE_PROJECT_NAME}.desktop")
set (XDG_AUTOSTART_FILE "${CMAKE_CURRENT_BINARY_DIR}/${XDG_AUTOSTART_NAME}")
set (XDG_AUTOSTART_FILE_IN "${CMAKE_CURRENT_SOURCE_DIR}/${XDG_AUTOSTART_NAME}.in")

# build it
set (pkglibexecdir "${CMAKE_INSTALL_FULL_PKGLIBEXECDIR}")
configure_file ("${XDG_AUTOSTART_FILE_IN}" "${XDG_AUTOSTART_FILE}")

# install it
install (FILES "${XDG_AUTOSTART_FILE}"
         DESTINATION "${XDG_AUTOSTART_DIR}")

###########################
# Upstart XDG Autostart Override
###########################

# where to install
set (UPSTART_XDG_AUTOSTART_DIR "${CMAKE_INSTALL_FULL_DATAROOTDIR}/upstart/xdg/autostart")
message (STATUS "${UPSTART_XDG_AUTOSTART_DIR} is the Upstart XDG autostart override dir")

set (UPSTART_XDG_AUTOSTART_NAME "${CMAKE_PROJECT_NAME}.upstart.desktop")
set (UPSTART_XDG_AUTOSTART_FILE "${CMAKE_CURRENT_BINARY_DIR}/${UPSTART_XDG_AUTOSTART_NAME}")
set (UPSTART_XDG_AUTOSTART_FILE_IN "${CMAKE_CURRENT_SOURCE_DIR}/${UPSTART_XDG_AUTOSTART_NAME}.in")

# build it
set (pkglibexecdir "${CMAKE_INSTALL_FULL_PKGLIBEXECDIR}")
configure_file ("${UPSTART_XDG_AUTOSTART_FILE_IN}" "${UPSTART_XDG_AUTOSTART_FILE}")

# install it
install (FILES "${UPSTART_XDG_AUTOSTART_FILE}"
         DESTINATION "${UPSTART_XDG_AUTOSTART_DIR}"
         RENAME "${XDG_AUTOSTART_NAME}")

