
Usage: relay_k[num] [-para index] [total time] [high time]
[num] 1,2,3,4.k1 and k2 use to USB,k3 k4 used to switch power by default
        if you want to change k1 and k2 to switch,P5,P6,P7jumpper 
        connect to P5,P6,P7 character side
  [-para] support two paramter 
        -s k3 k4 two channels switch by smae time
        -m specific multi pulses,max number 10
        index number of the specific pulse
  [total time] time of the period,1ms
  [high time] time of the pulse,1ms
Others:
  - CTRL + C: stop 
  - BackSpace: delete input
Examples:
  switch USB,period 1000ms,active time 500ms,another word duty cycles 0.5:
      relay_k1 1000 500
  switch k3,period 1000ms,active time 300ms,duty cycles 0.3:
      relay_k3 1000 300
  at the same time switch k3 k4,period 1000ms,active time 500ms,duty cycles 0.5:
      relay_k3 -s 1000 500
  specific multi pulses,"_-__--___---":
      relay_k3 -m 3 1000 500 2000 1000 3000 1500