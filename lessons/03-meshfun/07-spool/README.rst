#===============================================================================
# 07-spool (spooler based client/server application for reliable transmission)
#===============================================================================

- reliable client/server mesh communication by repeated sending of mesh messages
- repeating of mesh messages is done by bl_spool module (mesh publisher), which
  multiply schedules mesh commands with different delays
- addiditonally app supports mesh node house keeping functionality supported by
  bl_node module

# Lessons to Learn

- how to integrate and use the bl_spool() function
- how to send enhanced generic on/off commands utilizing the data reference part
  of the [GONOFF:SET @ix,<BL_goo>,onoff] command
- how to define transition objects
- how to deal with transitions using bl_trans(), bl_cur() and bl_fin()
- how to integrate app level library modules using bl_top()
