import win32com.client as comclt
import win32api

# Initialiseer de Windows shell in Python zodat je naar de shell kunt scripten
wsh= comclt.Dispatch("WScript.Shell")



# Voor elke keer dat je een opdracht wilt versturen:
wsh.AppActivate("Photo Gallery") # select another application
wsh.SendKeys("^.")


"""
SendKeys: next Photo   = wsh.SendKeys("{Right}")
          prev Photo   = wsh.SendKeys("{Left}")
          Rotate left  = wsh.SendKeys("^.")
          Rotate Right = wsh.SendKeys("^,")

"""

