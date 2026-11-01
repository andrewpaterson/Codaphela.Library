  // Tickable Pins
package net.simulation.common;

import net.common.*;
import net.util.StringUtil;

import java.util.ArrayList;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Set;

public abstract class TickablePins<
    SNAPSHOT extends Snapshot,
    PINS extends Pins<SNAPSHOT, PINS, ? extends IntegratedCircuit<SNAPSHOT, PINS>>,
    INTEGRATED_CIRCUIT extends IntegratedCircuit<SNAPSHOT, PINS>>
{
  protected INTEGRATED_CIRCUIT integratedCircuit;
  protected SNAPSHOT snapshot;

  protected Tickables tickables;
  protected List<Port> ports;

  protected long tickCount;

  public TickablePins(Tickables tickables)
  {
    this.tickables = tickables;
    this.tickables.add(this);
    this.ports = new ArrayList<>();
    this.tickCount = 1;
  }

  public void addPort(Port port)
  {
    ports.add(port);
  }

  public final void startPropagation()
  {
    snapshot = integratedCircuit.createSnapshot();
    getIntegratedCircuit().startTick();
  }

  public void propagate()
  {
    undoPropagation();
    getIntegratedCircuit().tick();
  }

  public final void undoPropagation()
  {
    if (snapshot != null)
    {
      integratedCircuit.restoreFromSnapshot(snapshot);
    }
  }

  public final void donePropagation()
  {
    snapshot = null;
    tickCount++;
  }

  public List<TraceValue> getTraceValues()
  {
    List<TraceValue> traceValues = new ArrayList<>();
    for (Port port : ports)
    {
      port.addTraceValues(traceValues);
    }
    return traceValues;
  }

  public void updateConnections()
  {
    for (Port port : ports)
    {
      port.updateConnection();
    }
  }

  public void resetConnections()
  {
    for (Port port : ports)
    {
      port.resetConnections();
    }
  }

  public String getName()
  {
    return getIntegratedCircuit().getName();
  }

  public String getDescription()
  {
    String name = getName();
    if (StringUtil.isEmptyOrNull(name))
    {
      return getType();
    }
    else
    {
      return getType() + " \"" + name + "\"";
    }
  }

  protected BusValue getBusValue(Omniport omniport)
  {
    TraceValue value = omniport.read();
    if (value.isError())
    {
      return BusValue.error();
    }
    else if (value.isNotConnected())
    {
      return BusValue.notConnected();
    }
    else if (value.isUnsettled())
    {
      return BusValue.unknown();
    }
    else
    {
      return new BusValue(omniport.getPinsAsBoolAfterRead());
    }
  }

  protected PinValue getPinValue(Uniport uniport)
  {
    TraceValue value = uniport.read();
    return getPinValue(value);
  }

  protected PinValue getPinValue(TraceValue value)
  {
    if (value.isError())
    {
      return PinValue.Error;
    }
    else if (value.isNotConnected())
    {
      return PinValue.NotConnected;
    }
    else if (value.isUnsettled())
    {
      return PinValue.Unknown;
    }
    else
    {
      return value.isHigh() ? PinValue.High : PinValue.Low;
    }
  }

  @Override
  public String toString()
  {
    return getDescription();
  }

  public String toDebugString()
  {
    StringBuilder stringBuilder = new StringBuilder();
    String description = " " + getDescription() + " ";
    stringBuilder.append(" ").append(StringUtil.centerJustify(description, 48, "-")).append(" \n");
    for (Port port : ports)
    {
      String portTransmissionState = StringUtil.rightJustify(port.getPortTransmissionStateAsString(), 24, " ");
      String portValues = port.getTraceValuesAsString();
      String connectionValues = port.getConnectionValuesAsString();
      if (portValues.equals(connectionValues))
      {
        stringBuilder.append(portTransmissionState).append(": P&W").append(portValues);
      }
      else
      {
        stringBuilder.append(portTransmissionState).append(":   P").append(portValues).append(" W");
        stringBuilder.append(connectionValues);
      }
      List<Trace> connections = port.getConnections();
      Set<Port> updatingPorts = get_DEBUG_UpdatingPorts(connections);
      if (updatingPorts.size() > 1)
      {
        stringBuilder.append(" (Multiple updaters)");
      }
      else if (updatingPorts.size() == 1)
      {
        stringBuilder.append(" (").append(updatingPorts.iterator().next().getDescription()).append(")");
      }

      stringBuilder.append("\n");
    }
    return stringBuilder.toString();
  }

  private Set<Port> get_DEBUG_UpdatingPorts(List<Trace> connections)
  {
    Set<Port> updatingPorts = new LinkedHashSet<>();
    for (Trace connection : connections)
    {
      Port updatingPort = connection.get_DEBUG_lastPortThatUpdated();
      if (updatingPort != null)
      {
        updatingPorts.add(updatingPort);
      }
    }
    return updatingPorts;
  }

  public INTEGRATED_CIRCUIT getIntegratedCircuit()
  {
    return integratedCircuit;
  }

  public void setIntegratedCircuit(INTEGRATED_CIRCUIT integratedCircuit)
  {
    this.integratedCircuit = integratedCircuit;
  }

  public String getType()
  {
    return integratedCircuit.getType();
  }

  public long getTickCount()
  {
    return tickCount;
  }
}


