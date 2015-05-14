PersoSim - Der Open Source Simulator für den elektronischen Personalausweis


*Installationsanforderungen*
- Für das Programm VirtualPerso wird mindestens .NET in Version 3.5 benötigt.

*Installation*
Bitte wählen Sie den Treiber, der zu Ihrer Rechnerarchitektur passt (32 oder 64 Bit). Sie können die Treiber sowohl für Win7 als auch für Win8 verwenden. Unter Win 8 benötigen Sie auf jeden Fall auch ein Zertifikat, um den Treiber zu installieren. Unter Win7 ist dieses Zertifikat optional. Sie installieren das Zertifkat mit dem entsprechenden Installer (PersoSim_Test_Certificates.msi). Anschließend installieren Sie bitte den Treiber für den virtuellen Kartenleser und folgen den Anweisungen des Installers. Nach erfolgreicher Installation erscheint der virtuelle Treiber als "PersoSim Virtual Reader" im Gerätemanager unter Smartcard-Leser.

*Start des virtuellen Personalausweises*
Starten Sie das Programm VirtualPerso.exe mit dem Sie den virtuellen Personalausweis "starten" und "beenden" können. Zu diesem Zweck dient das Häkchen "Card present". In der Box werden alle APDUS, die der virtuelle Treiber empfängt, aufgeführt. All diese Kommandos werden vom Simulator ausgeführt und erzeugen eine entsprechende Reaktion als Antwort-APDU bzw. als Statuswort.


Bei Fragen können Sie sich an den PersoSim-Support wenden unter der Adresse support@persosim.de

Wir wünschen Ihnen viel Spaß mit dem Simulator!
Das PersoSim-Team 


Lizenz:
Das Projekt PersoSim wird unter der GNU GPLv3 bereitgestellt.
Der Windows Treiber basiert auf einem Code-Sample von Microsoft zu virtuellen Treibern und dem Projekt "An UMDF Driver for a Virtual Smart Card Reader" von Fabio Ottavi (siehe http://www.codeproject.com/Articles/134010/An-UMDF-Driver-for-a-Virtual-Smart-Card-Reader). 