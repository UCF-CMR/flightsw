import os, sys, glob
import numpy as np
import matplotlib.pyplot as plt
from New_Shepard_Utilities import parse_flight_profile
from ERIE_Utilities import parse_electrometer_data

colors = plt.rcParams['axes.prop_cycle'].by_key()['color']

omit_events = ["INITIALIZE", "12V_REG_ENABLE", "ELECTROMETER_ENABLE", "STEPPER_RUN", "ELECTROMETER_DISABLE", "12V_REG_DISABLE", "TERMINATE"]

title = True
error = False
connect = True
profile = "PROFILE.TXT"

if len(sys.argv) > 1 and os.path.exists(sys.argv[1]):
    fn = sys.argv[1]
else:
    fn = sorted(glob.glob("[0-9]**.TXT"))[-1]

phead, pdata = parse_flight_profile(profile)

sensors = ["Teflon™ (––) 1", "Teflon™ (––) 2", "Garolite™ ( +) 1", "Garolite™ ( +) 2", "Lucite™ (++) 1", "Lucite™ (++) 2", "Lexan™ ( –) 1", "Lexan™ ( –) 2"]

events, C, T, V, S = parse_electrometer_data(fn)

cmax = max(C)
ymax = max([max(V[c]) for c in range(cmax + 1)])
ymin = min([min(V[c]) for c in range(cmax + 1)])

print("\nPayload events:")
em_disable_times = []
stepper_run_count = 0
fig, ax1 = plt.subplots(1, 1, figsize=(16,9))
for time, data in events:
    if data.startswith("Transitioning to state "):
        text = data[len("Transitioning to state "):]
        if text == "ELECTROMETER_DISABLE": em_disable_times.append(time/1000.)
        if not text.startswith("DELAY") and not text.endswith("_WAIT") and not text.endswith("_VERIFY") and text not in omit_events:
            ax1.axvline(time/1000., linestyle=':', linewidth=0.75, color="gray")
            ax1.annotate(text, (time/1000., ymin), rotation=90, color="gray")
    elif data.startswith("Stepper executed "):
        stepper_run_count += 1
        ax1.axvline(time/1000., linestyle=':', linewidth=0.75, color="gray")
        if stepper_run_count == 2: ha = "left"    # TODO: MAKE THIS CLEVER BY DETECTING NEARBY EVENTS
        else:                      ha = "right"
        ax1.annotate("STEPPER_RUN_COMPLETE", (time/1000., ymin), ha=ha, rotation=90, color="gray")
    print("  %{:}.3f: %s".format(len(str(events[-1][0]))+1) % (time/1000., data))

if pdata:
    print("\nVehicle profile events:")
    for time, data in pdata:
        ax1.axvline(time, linestyle=':', linewidth=0.75, color="red")
        ax1.annotate(data.upper().replace('/', '\n'), (time, ymin), rotation=90, color="red")
        print("%5d: %s" % (time, data))

Tarr = [None]*(cmax + 1)
Varr = [None]*(cmax + 1)
Sarr = [None]*(cmax + 1)
for c in range(cmax + 1):
    Tarr[c] = np.array(T[c])
    Varr[c] = np.array(V[c])
    Sarr[c] = np.array(S[c])
    if len(em_disable_times) > 1:
        if c == 0: print("\nElectrometer discontinuities:")
        for em_disable_time in em_disable_times[:-1]:
            if c == 0: print("  %.3f" % em_disable_time)
            idx = np.where(Tarr[c] > em_disable_time)[0][0]
            Tarr[c] = np.insert(Tarr[c], idx, em_disable_time)
            Varr[c] = np.insert(Varr[c], idx, np.nan)
    if c % 2 == c % 2: #1: # TODO: REMOVE EVEN/ODD CHECK
        if error:
            if connect:
                ax1.errorbar(Tarr[c], Varr[c], Sarr[c], label=sensors[c], color=colors[c], fmt=".", ms=2., capsize=2., errorevery=np.ceil(len(T[0])/100))
            else:
                ax1.errorbar(Tarr[c], Varr[c], Sarr[c], label=sensors[c], color=colors[c], fmt=".", ms=2., ls="none", capsize=2., errorevery=np.ceil(len(T[0])/100))
        else:
            if connect:
                ax1.plot(Tarr[c], Varr[c], label=sensors[c], color=colors[c])
            else:
                ax1.scatter(Tarr[c], Varr[c], label=sensors[c], color=colors[c], s=2.)

ax1.set_xlabel("Elapsed Time (s)")
ax1.set_ylabel("Sensor Output (V)")
if title: plt.title(fn)
plt.tight_layout()
box = plt.gca().get_position()
plt.gca().set_position([box.x0, box.y0 + box.height * 0.05, box.width, box.height * 0.95])
# Uncomment for single row legend
plt.legend(loc='upper center', bbox_to_anchor=(0.5, -0.07), ncol=8)
# Uncomment for double row legend
#plt.legend(loc='upper center', bbox_to_anchor=(0.5, -0.05), ncol=4)
plt.savefig(fn.replace(".TXT", ".png").replace(".txt", ".png"), dpi=150)

plt.show()
