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



class EnergyPlotter:

    def __init__(self,app):
        self.app=app
        self.x_values = []
        self.y_values = []
        self.cpu_y_values = []
        self.sd_y_values = []
        self.nic_y_values = []
        # Define the size of the window to display the most recent data points
        self.window_size = 20  # Adjust this to show more or fewer points


    ###################################################################################
    ##       PANEL LAYOUT 
    ###################################################################################



    def energy_plotter_layout(self):
        return html.Div([
            html.Div([
                html.H3("FLOC: Made by DragonKlavier"),
                html.Label("Update Interval (ms):"),
                dcc.Input(id='energy-interval-setting', type='number', value=1000, placeholder="Enter update interval in milliseconds"),
                html.Button('Start Plotting', id='energy-start-plotting', className="button", n_clicks=0),
                html.Button('Stop Plotting', id='energy-stop-plotting', className="button", n_clicks=0),
                dcc.Interval(id='update-energy-interval', interval=1000, n_intervals=0, disabled=True),
            ], style={'width': '30%', 'display': 'inline-block', 'vertical-align': 'top'}),

            html.Div([
                dcc.Graph(id='energy-line-chart'),
                dcc.Graph(id='energy-pie-chart')  # Placeholder for the pie chart
            ], style={'width': '69%', 'display': 'inline-block', 'padding-left': '20px'}),
        ])



    ###################################################################################
    ##      CORE FUNCTIONS
    ###################################################################################


    def get_CPU_total_power(self):

        cpu_power = 0.0  
        try:
            with open('/dev/shm/CPU_shared_memory', 'r') as file:
                lines = file.readlines()
                for line in lines:
                    if "total_power=" in line:
                        cpu_power = float(line.split('=')[1].strip())
                        return cpu_power
        except Exception as e:
            print(f"Failed to read from /dev/shm/CPU_shared_memory: {e}")
            return cpu_power


    def get_SD_total_power(self):

        sd_power = 0.0
        try:
            with open('/dev/shm/SD_shared_memory', 'r') as file:
                lines = file.readlines()
                for line in lines:
                    if "total_power=" in line:
                        sd_power = float(line.split('=')[1].strip())
                        return sd_power
        except Exception as e:
            print(f"Failed to read from /dev/shm/SD_shared_memory: {e}")
            return sd_power

    def get_NIC_total_power(self):

        nic_power = 0.0  
        try:
            with open('/dev/shm/NIC_shared_memory', 'r') as file:
                lines = file.readlines()
                for line in lines:
                    if "total_power=" in line:
                        nic_power = float(line.split('=')[1].strip())
                        return nic_power
        except Exception as e:
            print(f"Failed to read from /dev/shm/NIC_shared_memory: {e}")
            return nic_power
        


    ###################################################################################
    ##      DECORATORS AND CALLBACKS: CURRENT POWER 
    ###################################################################################



    def register_energy_plotter_callbacks(self):
    
        @self.app.callback(
            [Output('update-energy-interval', 'interval'),
            Output('update-energy-interval', 'disabled')],
            [Input('energy-start-plotting', 'n_clicks'),
            Input('energy-stop-plotting', 'n_clicks')],
            [State('energy-interval-setting', 'value')]
        )
        def update_interval_settings(start_n_clicks, stop_n_clicks, interval_value):
            if not start_n_clicks and not stop_n_clicks:
                raise PreventUpdate

            triggered_id = dash.callback_context.triggered[0]['prop_id'].split('.')[0]
            if triggered_id == 'energy-start-plotting' and interval_value is not None:
                self.x_values.clear()
                self.y_values.clear()
                self.cpu_y_values.clear()
                self.sd_y_values.clear()
                self.nic_y_values.clear()
                return interval_value, False  # Enable and set interval
            elif triggered_id == 'energy-stop-plotting':
                return dash.no_update, True  # Disable without changing the interval
            return 5000, True  # Default settings

        # Update Plot Function for Line and Pie Charts
        @self.app.callback(
            [Output('energy-line-chart', 'figure'),
            Output('energy-pie-chart', 'figure')],
            [Input('update-energy-interval', 'n_intervals')],
            prevent_initial_call=True
        )
        def update_graph_live(n):
            # Append new data points and maintain window size
            self.x_values.append(n)
            self.sd_y_values.append(self.get_SD_total_power())
            self.cpu_y_values.append(self.get_CPU_total_power())
            self.nic_y_values.append(self.get_NIC_total_power())

            if len(self.x_values) > self.window_size:
                self.x_values.pop(0)
                self.sd_y_values.pop(0)
                self.cpu_y_values.pop(0)

            # Create the line chart figure
            line_chart_figure = go.Figure()
            line_chart_figure.add_trace(go.Scatter(x=self.x_values, y=self.sd_y_values, 
                                                   mode='lines+markers', name='SD Energy'))
            line_chart_figure.add_trace(go.Scatter(x=self.x_values, y=self.cpu_y_values, 
                                                   mode='lines+markers', name='CPU Energy'))
            line_chart_figure.add_trace(go.Scatter(x=self.x_values, y=self.nic_y_values,
                                                    mode='lines+markers', name='NIC Power'))
            line_chart_figure.update_layout(title='Device Power Consumption', 
                                            xaxis_title='Time', yaxis_title='Power (Watts)', 
                                            legend_title='Power Sources')

            # Create the pie chart figure
            sd_power = self.sd_y_values[-1] if self.sd_y_values else 0
            cpu_power = self.cpu_y_values[-1] if self.cpu_y_values else 0
            nic_power = self.nic_y_values[-1] if self.nic_y_values else 0
            pie_chart_figure = go.Figure(data=[go.Pie(labels=['SD Power', 'CPU Power', 'NIC Power'], values=[sd_power, cpu_power, nic_power], hole=.4)])
            #pie_chart_figure = go.Figure(data=[go.Pie(labels=['SD Energy', 'CPU Energy'], 
             #                                         values=[sd_power, cpu_power], hole=.4)])
            pie_chart_figure.update_layout(title='Power Consumption Percentage')

            return line_chart_figure, pie_chart_figure






