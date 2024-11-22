# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at

#   http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.



import dash
from dash import html, dcc, callback_context
import dash_bootstrap_components as dbc
from dash.dependencies import Input, Output, State
from dash.exceptions import PreventUpdate
import plotly.graph_objs as go

#the shared variable names 
import shared_vars


#for shared memory management
import mmap
import posix_ipc
import struct


# Initialize global lists to store x and y values for the graph
x_power_values = []
x_energy_values = []


y_power_cpu_values = []
y_power_ram_values = []
y_power_nic_values = []
y_power_sd_values = []


y_energy_cpu_values = []
y_energy_ram_values = []
y_energy_nic_values = []
y_energy_sd_values = []


# Define the size of the window to display the most recent data points
window_size = 20  # Adjust this to show more or fewer points


###################################################################################
##       PANEL LAYOUT 
###################################################################################



def power_plotter_layout():
    return html.Div([
        html.Div([
            html.H3("Update Interval (ms):"),
            dcc.Input(id='power-interval-setting', type='number', value=1000, placeholder="Enter update interval in milliseconds"),

            # Arrange buttons horizontally with flexbox
            html.Div([
                html.Button('Start Plotting', id='start-plotting', className="button", n_clicks=0),
                html.Button('Stop Plotting', id='stop-plotting', className="button", n_clicks=0),
            ], style={'display': 'flex'}),  # Align buttons to the right

            dcc.Interval(id='update-power-interval', interval=1000, n_intervals=0, disabled=True),
        ], style={'width': '30%', 'display': 'inline-block', 'vertical-align': 'top'}),



        html.H3("CURRENT POWER CONSUMPTION", style={'text-align': 'center'}),



        html.Div([
            html.Div([
                dcc.Graph(id='histogram-current-power', figure={})  # Placeholder for the histogram
            ], style={'width': '40%','hight': '20%', 'display': 'inline-block'}),

            html.Div([
                dcc.Graph(id='line-graph-current-power', figure={})  # Placeholder for the line graph
            ], style={'width': '40%','hight': '20%', 'display': 'inline-block', 'padding-left': '20px'}),
        ], style={'display': 'flex'}),

        
        html.H3("CUMULATED ENERGY CONSUMPTION", style={'text-align': 'center'}),

        html.Div([
            html.Div([
                dcc.Graph(id='pie-chart-energy', figure={})  # Placeholder for the pie chart
            ], style={'width': '40%','hight': '20%', 'display': 'inline-block'}),

            html.Div([
                dcc.Graph(id='line-chart-cumulated-energy', figure={})  # Placeholder for the line chart
            ], style={'width': '40%','hight': '20%', 'display': 'inline-block', 'padding-left': '20px'}),
        ], style={'display': 'flex'})
    ])



###################################################################################
##      CORE FUNCTIONS
###################################################################################

#TODO -> call this functions only if selected in the launch daemon tab


#######################CURRENT POWER##########################################



def get_CPU_current_power():
    SHARED_MEMORY_PATH = f"/dev/shm/{shared_vars.shared_cpu}"
    SHARED_MEMORY_SIZE = 288  # Total size of the structure

    try:
        with open(SHARED_MEMORY_PATH, "rb") as shm_file:
            with mmap.mmap(shm_file.fileno(), SHARED_MEMORY_SIZE, access=mmap.ACCESS_READ) as shm:
                fmt = "=256s i d d Q i"  # Corrected format string
                unpacked_data = struct.unpack(fmt, shm.read(SHARED_MEMORY_SIZE))

                # Extract average_power (index 2 after corrections)
                average_power = unpacked_data[2]

                return average_power
    except FileNotFoundError:
        print(f"Shared memory file not found: {SHARED_MEMORY_PATH}")
    except Exception as e:
        print(f"Error reading shared memory: {e}")

    return 0.0  # Default value if reading fails



def get_RAM_current_power():
    SHARED_MEMORY_PATH = f"/dev/shm/{shared_vars.shared_ram}"
    SHARED_MEMORY_SIZE = 288  # Total size of the structure

    try:
        with open(SHARED_MEMORY_PATH, "rb") as shm_file:
            with mmap.mmap(shm_file.fileno(), SHARED_MEMORY_SIZE, access=mmap.ACCESS_READ) as shm:
                fmt = "=256s i d d Q i"  # Corrected format string
                unpacked_data = struct.unpack(fmt, shm.read(SHARED_MEMORY_SIZE))

                # Extract average_power (index 2 after corrections)
                average_power = unpacked_data[2]

                return average_power
    except FileNotFoundError:
        print(f"Shared memory file not found: {SHARED_MEMORY_PATH}")
    except Exception as e:
        print(f"Error reading shared memory: {e}")

    return 0.0  # Default value if reading fails


def get_SD_current_power():
    SHARED_MEMORY_PATH = f"/dev/shm/{shared_vars.shared_sd}"
    SHARED_MEMORY_SIZE = 288  # Total size of the structure

    try:
        with open(SHARED_MEMORY_PATH, "rb") as shm_file:
            with mmap.mmap(shm_file.fileno(), SHARED_MEMORY_SIZE, access=mmap.ACCESS_READ) as shm:
                fmt = "=256s i d d Q i"  # Corrected format string
                unpacked_data = struct.unpack(fmt, shm.read(SHARED_MEMORY_SIZE))

                # Extract average_power (index 2 after corrections)
                average_power = unpacked_data[2]

                return average_power
    except FileNotFoundError:
        print(f"Shared memory file not found: {SHARED_MEMORY_PATH}")
    except Exception as e:
        print(f"Error reading shared memory: {e}")

    return 0.0  # Default value if reading fails


def get_NIC_current_power():
    SHARED_MEMORY_PATH = f"/dev/shm/{shared_vars.shared_nic}"
    SHARED_MEMORY_SIZE = 288  # Total size of the structure

    try:
        with open(SHARED_MEMORY_PATH, "rb") as shm_file:
            with mmap.mmap(shm_file.fileno(), SHARED_MEMORY_SIZE, access=mmap.ACCESS_READ) as shm:
                fmt = "=256s i d d Q i"  # Corrected format string
                unpacked_data = struct.unpack(fmt, shm.read(SHARED_MEMORY_SIZE))

                # Extract average_power (index 2 after corrections)
                average_power = unpacked_data[2]

                return average_power
    except FileNotFoundError:
        print(f"Shared memory file not found: {SHARED_MEMORY_PATH}")
    except Exception as e:
        print(f"Error reading shared memory: {e}")

    return 0.0  # Default value if reading fails




#######################CUMULATIVE ENERGY##########################################



def get_CPU_cumulated_energy():
    SHARED_MEMORY_PATH = f"/dev/shm/{shared_vars.shared_cpu}"
    SHARED_MEMORY_SIZE = 288

    try:
        with open(SHARED_MEMORY_PATH, "rb") as shm_file:
            with mmap.mmap(shm_file.fileno(), SHARED_MEMORY_SIZE, access=mmap.ACCESS_READ) as shm:
                fmt = "=256s i d d Q i"
                unpacked_data = struct.unpack(fmt, shm.read(SHARED_MEMORY_SIZE))

                total_energy = unpacked_data[3]  # Total energy is at index 3
                return total_energy
    except FileNotFoundError:
        print(f"Shared memory file not found: {SHARED_MEMORY_PATH}")
    except Exception as e:
        print(f"Error reading shared memory: {e}")

    return 0.0


def get_RAM_cumulated_energy():
    SHARED_MEMORY_PATH = f"/dev/shm/{shared_vars.shared_ram}"
    SHARED_MEMORY_SIZE = 288

    try:
        with open(SHARED_MEMORY_PATH, "rb") as shm_file:
            with mmap.mmap(shm_file.fileno(), SHARED_MEMORY_SIZE, access=mmap.ACCESS_READ) as shm:
                fmt = "=256s i d d Q i"
                unpacked_data = struct.unpack(fmt, shm.read(SHARED_MEMORY_SIZE))

                total_energy = unpacked_data[3]
                return total_energy
    except FileNotFoundError:
        print(f"Shared memory file not found: {SHARED_MEMORY_PATH}")
    except Exception as e:
        print(f"Error reading shared memory: {e}")

    return 0.0


def get_SD_cumulated_energy():
    SHARED_MEMORY_PATH = f"/dev/shm/{shared_vars.shared_sd}"
    SHARED_MEMORY_SIZE = 288

    try:
        with open(SHARED_MEMORY_PATH, "rb") as shm_file:
            with mmap.mmap(shm_file.fileno(), SHARED_MEMORY_SIZE, access=mmap.ACCESS_READ) as shm:
                fmt = "=256s i d d Q i"
                unpacked_data = struct.unpack(fmt, shm.read(SHARED_MEMORY_SIZE))

                total_energy = unpacked_data[3]
                return total_energy
    except FileNotFoundError:
        print(f"Shared memory file not found: {SHARED_MEMORY_PATH}")
    except Exception as e:
        print(f"Error reading shared memory: {e}")

    return 0.0


def get_NIC_cumulated_energy():
    SHARED_MEMORY_PATH = f"/dev/shm/{shared_vars.shared_nic}"
    SHARED_MEMORY_SIZE = 288

    try:
        with open(SHARED_MEMORY_PATH, "rb") as shm_file:
            with mmap.mmap(shm_file.fileno(), SHARED_MEMORY_SIZE, access=mmap.ACCESS_READ) as shm:
                fmt = "=256s i d d Q i"
                unpacked_data = struct.unpack(fmt, shm.read(SHARED_MEMORY_SIZE))

                total_energy = unpacked_data[3]
                return total_energy
    except FileNotFoundError:
        print(f"Shared memory file not found: {SHARED_MEMORY_PATH}")
    except Exception as e:
        print(f"Error reading shared memory: {e}")

    return 0.0


###################################################################################
##      DECORATORS AND CALLBACKS: CURRENT POWER 
###################################################################################



def register_power_plotter_callbacks(app):
 
    @app.callback(
        [Output('update-power-interval', 'interval'),
         Output('update-power-interval', 'disabled')],
        [Input('start-plotting', 'n_clicks'),
         Input('stop-plotting', 'n_clicks')],
        [State('power-interval-setting', 'value')]
    )
    def update_interval_settings(start_n_clicks, stop_n_clicks, interval_value):
        if not start_n_clicks and not stop_n_clicks:
            raise PreventUpdate

        triggered_id = dash.callback_context.triggered[0]['prop_id'].split('.')[0]
        if triggered_id == 'start-plotting' and interval_value is not None:
            x_power_values.clear()
            x_energy_values.clear()


            y_power_cpu_values.clear()
            y_power_ram_values.clear()
            y_power_nic_values.clear()
            y_power_sd_values.clear()


            y_energy_cpu_values.clear()
            y_energy_ram_values.clear()
            y_energy_nic_values.clear()
            y_energy_sd_values.clear()
            return interval_value, False  # Enable and set interval
        elif triggered_id == 'stop-plotting':
            return dash.no_update, True  # Disable without changing the interval
        return 5000, True  # Default settings



    # Update Plot Function for Line and Pie Charts

    @app.callback(
    [Output('line-graph-current-power', 'figure'),
     Output('pie-chart-energy', 'figure'),
     Output('histogram-current-power', 'figure'),
     Output('line-chart-cumulated-energy', 'figure')],
    [Input('update-power-interval', 'n_intervals')],
    prevent_initial_call=True
    )
    def update_graph_live(n):
        
        # Append new data points for power and maintain window size
        x_power_values.append(n)
        y_power_cpu_values.append(get_CPU_current_power())
        y_power_ram_values.append(get_RAM_current_power())
        y_power_sd_values.append(get_SD_current_power())
        y_power_nic_values.append(get_NIC_current_power())

        # Append new data points for energy
        x_energy_values.append(n)
        y_energy_cpu_values.append(get_CPU_cumulated_energy())
        y_energy_ram_values.append(get_RAM_cumulated_energy())
        y_energy_nic_values.append(get_NIC_cumulated_energy())
        y_energy_sd_values.append(get_SD_cumulated_energy())

        # Maintain a fixed window size for the x-axis if needed
        window_size = 50  # Example window size
        if len(x_power_values) > window_size:
            x_power_values.pop(0)
            y_power_cpu_values.pop(0)
            y_power_ram_values.pop(0)
            y_power_sd_values.pop(0)
            y_power_nic_values.pop(0)

        if len(x_energy_values) > window_size:
            x_energy_values.pop(0)
            y_energy_cpu_values.pop(0)
            y_energy_ram_values.pop(0)
            y_energy_nic_values.pop(0)
            y_energy_sd_values.pop(0)

        # Define colors
        colors = {
            'CPU': 'blue',
            'RAM': 'green',
            'NIC': 'red',
            'SD': 'orange'
        }

        # Create the line chart for the current power
        line_power_figure = go.Figure()
        line_power_figure.add_trace(go.Scatter(x=x_power_values, y=y_power_cpu_values, mode='lines+markers', name='CPU Power', line=dict(color=colors['CPU'])))
        line_power_figure.add_trace(go.Scatter(x=x_power_values, y=y_power_ram_values, mode='lines+markers', name='RAM Power', line=dict(color=colors['RAM'])))
        line_power_figure.add_trace(go.Scatter(x=x_power_values, y=y_power_sd_values, mode='lines+markers', name='SD Power', line=dict(color=colors['SD'])))
        line_power_figure.add_trace(go.Scatter(x=x_power_values, y=y_power_nic_values, mode='lines+markers', name='NIC Power', line=dict(color=colors['NIC'])))
        line_power_figure.update_layout(title='Current Power Consumption', xaxis_title='Time', yaxis_title='Power (Watts)', legend_title='Components')

        # Create the pie chart for energy consumption, ensuring all parts sum to 100%
        pie_energy_figure = go.Figure(data=[go.Pie(labels=['CPU Energy', 'RAM Energy', 'NIC Energy', 'SD Energy'],
                                                values=[y_energy_cpu_values[-1], y_energy_ram_values[-1], y_energy_nic_values[-1], y_energy_sd_values[-1]],
                                                hole=.4, textinfo='label+percent', hoverinfo='label+percent+value',
                                                marker=dict(colors=[colors['CPU'], colors['RAM'], colors['NIC'], colors['SD']]))])
        pie_energy_figure.update_layout(title='Energy Consumption Percentage')

        # Create the histogram for the current power
        histogram_power_figure = go.Figure(data=[go.Bar(x=['CPU', 'RAM', 'SD', 'NIC'], 
                                                        y=[y_power_cpu_values[-1], y_power_ram_values[-1], y_power_sd_values[-1], y_power_nic_values[-1]],
                                                        marker_color=[colors['CPU'], colors['RAM'], colors['SD'], colors['NIC']])])
        histogram_power_figure.update_layout(title='Current Power Distribution', xaxis_title='Component', yaxis_title='Power (Watts)')

        # Create the line chart for the cumulated energy
        line_energy_figure = go.Figure()
        line_energy_figure.add_trace(go.Scatter(x=x_energy_values, y=y_energy_cpu_values, mode='lines+markers', name='CPU Energy', line=dict(color=colors['CPU'])))
        line_energy_figure.add_trace(go.Scatter(x=x_energy_values, y=y_energy_ram_values, mode='lines+markers', name='RAM Energy', line=dict(color=colors['RAM'])))
        line_energy_figure.add_trace(go.Scatter(x=x_energy_values, y=y_energy_sd_values, mode='lines+markers', name='SD Energy', line=dict(color=colors['SD'])))
        line_energy_figure.add_trace(go.Scatter(x=x_energy_values, y=y_energy_nic_values, mode='lines+markers', name='NIC Energy', line=dict(color=colors['NIC'])))
        line_energy_figure.update_layout(title='Cumulated Energy Consumption', xaxis_title='Time', yaxis_title='Energy (Watt-hours)', legend_title='Components')

        return line_power_figure, pie_energy_figure, histogram_power_figure, line_energy_figure
