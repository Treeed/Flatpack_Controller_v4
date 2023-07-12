# Flatpack_Controller_v4
Controller for Flatpack 48-60/2000 HE using HX8347 Touchscreen shield on an Arduino Uno with MCP2515 CAN controller which allows it to operate as a flexible 10-18S battery charger. \
Can be easily adapted for other displays by replacing TFT_HX8347 with another one of Bodmer's libraries, e.g. TFT_ILI9341.


If you use this, make sure to be aware of the limitations of the flatpack as a battery charger (i.e. lowest voltage limit)! I'm not responsible if you burn down your LiPo or more! This software does not implement any safeguarding against unsafe use.

Also includes a library "FlatpackMCP" which allows simple control of a single Flatpack 2 in every way that is currently publicly known

pic of interface:\
<img src="https://github.com/Treeed/Flatpack_Controller_v4/assets/21248276/c5303e88-2f6a-455b-939a-1217c402cd60" alt="drawing" width="200"/>
